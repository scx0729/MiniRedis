#include "miniredis/resp/parser.h"
#include <algorithm>
#include <charconv>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>

namespace miniredis {

RespParser::RespParser(ParserLimits limits) : limits_(limits) {}

void RespParser::Reset() {
    input_buffer_.clear();
}

bool RespParser::HasBufferedData() const {
    return !input_buffer_.empty();
}

size_t RespParser::FindCRLF(std::string_view input, size_t offset) const {
    for (size_t i = offset; i + 1 < input.size(); ++i) {
        if (input[i] == '\r' && input[i + 1] == '\n') {
            return i;
        }
    }
    return std::string_view::npos;
}

Result<int64_t> RespParser::ParseInt(std::string_view s) const {
    if (s.empty()) return Result<int64_t>::Err(ErrorCode::kProtocolError, "empty integer");

    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec != std::errc() || ptr != s.data() + s.size()) {
        return Result<int64_t>::Err(ErrorCode::kProtocolError, "invalid integer");
    }
    return Result<int64_t>::Ok(val);
}

// Parse a single RESP value from input starting at pos.
// Returns the result and updates pos to point past the consumed bytes.
// This function does NOT modify line_buffer_ - it works purely on the input view.
static ParseResult ParseOne(std::string_view input, size_t& pos, size_t depth,
                            const ParserLimits& limits,
                            RespParser* /*parser*/) {
    if (depth > limits.max_depth) {
        return ParseResult::ProtocolError(
            Error(ErrorCode::kMaxDepthExceeded, "max nesting depth exceeded"));
    }

    if (pos >= input.size()) {
        return ParseResult::NeedMoreData();
    }

    char prefix = input[pos];

    // Find CRLF
    size_t crlf = std::string_view::npos;
    for (size_t i = pos + 1; i + 1 < input.size(); ++i) {
        if (input[i] == '\r' && input[i + 1] == '\n') {
            crlf = i;
            break;
        }
    }
    if (crlf == std::string_view::npos) {
        return ParseResult::NeedMoreData();
    }

    std::string_view line = input.substr(pos + 1, crlf - pos - 1);

    switch (prefix) {
        case '+': {
            std::string value(line);
            size_t consumed = crlf + 2 - pos;
            pos = crlf + 2;
            return ParseResult::Complete(consumed, RespValue::SimpleString(std::move(value)));
        }
        case '-': {
            std::string value(line);
            size_t consumed = crlf + 2 - pos;
            pos = crlf + 2;
            return ParseResult::Complete(consumed, RespValue::Error(std::move(value)));
        }
        case ':': {
            // Parse integer
            int64_t val = 0;
            auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), val);
            if (ec != std::errc() || ptr != line.data() + line.size()) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kProtocolError, "invalid integer"));
            }
            size_t consumed = crlf + 2 - pos;
            pos = crlf + 2;
            return ParseResult::Complete(consumed, RespValue::Integer(val));
        }
        case '$': {
            // Bulk String
            int64_t len = 0;
            auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), len);
            if (ec != std::errc() || ptr != line.data() + line.size()) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kProtocolError, "invalid bulk length"));
            }

            if (len < -1) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kInvalidBulkLength, "invalid bulk string length"));
            }

            if (len == -1) {
                size_t consumed = crlf + 2 - pos;
                pos = crlf + 2;
                return ParseResult::Complete(consumed, RespValue::NullBulkString());
            }

            if (static_cast<size_t>(len) > limits.max_bulk_length) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kInvalidBulkLength, "bulk string too long"));
            }

            size_t bulk_len = static_cast<size_t>(len);
            size_t data_start = crlf + 2;
            size_t available = input.size() - data_start;

            if (available < bulk_len + 2) {
                return ParseResult::NeedMoreData();
            }

            // Check CRLF after data
            if (input[data_start + bulk_len] != '\r' ||
                input[data_start + bulk_len + 1] != '\n') {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kProtocolError, "expected CRLF after bulk string"));
            }

            std::string value(input.substr(data_start, bulk_len));
            size_t consumed = data_start + bulk_len + 2 - pos;
            pos = data_start + bulk_len + 2;
            return ParseResult::Complete(consumed, RespValue::BulkString(std::move(value)));
        }
        case '*': {
            // Array
            int64_t count = 0;
            auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), count);
            if (ec != std::errc() || ptr != line.data() + line.size()) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kProtocolError, "invalid array length"));
            }

            if (count < -1) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kInvalidArrayLength, "invalid array length"));
            }

            if (count == -1) {
                size_t consumed = crlf + 2 - pos;
                pos = crlf + 2;
                return ParseResult::Complete(consumed, RespValue::NullArray());
            }

            if (static_cast<size_t>(count) > limits.max_array_length) {
                return ParseResult::ProtocolError(
                    Error(ErrorCode::kInvalidArrayLength, "array too long"));
            }

            size_t array_start = pos;
            pos = crlf + 2;

            RespValue::ArrayT arr;
            arr.reserve(static_cast<size_t>(count));

            for (int64_t i = 0; i < count; ++i) {
                auto elem_result = ParseOne(input, pos, depth + 1, limits, nullptr);
                if (elem_result.status == ParseStatus::kNeedMoreData) {
                    pos = array_start;  // reset pos
                    return ParseResult::NeedMoreData();
                }
                if (elem_result.status == ParseStatus::kProtocolError) {
                    pos = array_start;
                    return elem_result;
                }
                arr.push_back(std::move(elem_result.value));
            }

            size_t consumed = pos - array_start;
            return ParseResult::Complete(consumed, RespValue::MakeArray(std::move(arr)));
        }
        default:
            return ParseResult::ProtocolError(
                Error(ErrorCode::kProtocolError, "unknown RESP type prefix"));
    }
}

ParseResult RespParser::Parse(std::string_view input) {
    input_buffer_.append(input);

    const size_t max_buffer = limits_.max_bulk_length + limits_.max_array_length * 16 + 1024;
    if (input_buffer_.size() > max_buffer) {
        input_buffer_.clear();
        return ParseResult::ProtocolError(
            Error(ErrorCode::kMaxInputExceeded, "input too large"));
    }

    size_t pos = 0;
    auto result = ParseOne(input_buffer_, pos, 0, limits_, this);

    if (result.status == ParseStatus::kComplete) {
        input_buffer_.erase(0, pos);
    } else if (result.status == ParseStatus::kProtocolError) {
        input_buffer_.clear();
    }

    return result;
}

}  // namespace miniredis
