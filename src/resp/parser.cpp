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
    line_buffer_.clear();
    pending_bulk_bytes_ = 0;
    bulk_buffer_.clear();
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
    // Append input to buffer
    line_buffer_.append(input);

    // Check total size
    if (line_buffer_.size() > limits_.max_bulk_length * 2) {
        line_buffer_.clear();
        return ParseResult::ProtocolError(
            Error(ErrorCode::kMaxInputExceeded, "input too large"));
    }

    // Check for pending bulk string data from previous parse
    if (pending_bulk_bytes_ > 0) {
        size_t available = line_buffer_.size();
        if (available < pending_bulk_bytes_ + 2) {
            // Not enough for complete bulk + CRLF
            // But check if we have all the data and just need CRLF
            if (available >= pending_bulk_bytes_) {
                // We have all data bytes, need CRLF
                if (available < pending_bulk_bytes_ + 2) {
                    return ParseResult::NeedMoreData();
                }
            } else {
                return ParseResult::NeedMoreData();
            }
        }

        // Check CRLF after bulk data
        if (line_buffer_[pending_bulk_bytes_] != '\r' ||
            line_buffer_[pending_bulk_bytes_ + 1] != '\n') {
            line_buffer_.clear();
            pending_bulk_bytes_ = 0;
            return ParseResult::ProtocolError(
                Error(ErrorCode::kProtocolError, "expected CRLF after bulk string data"));
        }

        bulk_buffer_.append(line_buffer_.substr(0, pending_bulk_bytes_));
        size_t consumed = pending_bulk_bytes_ + 2;
        std::string result_str = std::move(bulk_buffer_);
        bulk_buffer_.clear();
        pending_bulk_bytes_ = 0;
        line_buffer_.erase(0, consumed);
        return ParseResult::Complete(consumed, RespValue::BulkString(std::move(result_str)));
    }

    // Try to parse one complete value
    size_t pos = 0;
    auto result = ParseOne(line_buffer_, pos, 0, limits_, this);

    if (result.status == ParseStatus::kNeedMoreData) {
        // Check if we have a partial bulk string situation
        // Look for $\r\n to detect partial bulk
        if (!line_buffer_.empty() && line_buffer_[0] == '$') {
            size_t crlf = FindCRLF(line_buffer_, 1);
            if (crlf != std::string_view::npos && crlf > 1) {
                // Parse bulk length manually
                std::string len_copy(line_buffer_.substr(1, crlf - 1));
                char* endp = nullptr;
                long long bulk_len_ll = strtoll(len_copy.c_str(), &endp, 10);
                if (*endp == '\0' && bulk_len_ll > 0) {
                    size_t bulk_len = static_cast<size_t>(bulk_len_ll);
                    size_t data_start = crlf + 2;
                    if (data_start < line_buffer_.size()) {
                        size_t available = line_buffer_.size() - data_start;
                        if (available < bulk_len) {
                            // We have partial bulk data
                            pending_bulk_bytes_ = bulk_len - available;
                            bulk_buffer_ = line_buffer_.substr(data_start, available);
                            line_buffer_.erase(0, data_start + available);
                            return ParseResult::NeedMoreData();
                        }
                    }
                }
            }
        }
        return ParseResult::NeedMoreData();
    }

    if (result.status == ParseStatus::kComplete) {
        line_buffer_.erase(0, pos);
    }

    return result;
}

}  // namespace miniredis
