// TODO(Task 2): implement an incremental RESP2 parser.
#include "miniredis/resp/parser.h"

namespace miniredis {
namespace {

RespValue::ArrayT PlaceholderArray(size_t count) {
    RespValue::ArrayT arr;
    for (size_t i = 0; i < count; ++i) {
        arr.push_back(RespValue::BulkString(""));
    }
    return arr;
}

RespValue::ArrayT PlaceholderCommand(std::string name, size_t argc) {
    RespValue::ArrayT arr;
    arr.push_back(RespValue::BulkString(std::move(name)));
    for (size_t i = 1; i < argc; ++i) {
        arr.push_back(RespValue::BulkString(""));
    }
    return arr;
}

}  // namespace

RespParser::RespParser(ParserLimits limits) : limits_(limits) {}

void RespParser::Reset() {
    input_buffer_.clear();
}

bool RespParser::HasBufferedData() const {
    return !input_buffer_.empty();
}

size_t RespParser::FindCRLF([[maybe_unused]] std::string_view input, [[maybe_unused]] size_t offset) const {
    // TODO(Task 2): find a CRLF line terminator.
    return std::string_view::npos;
}

Result<int64_t> RespParser::ParseInt([[maybe_unused]] std::string_view text) const {
    // TODO(Task 2): parse signed decimal integers without accepting junk suffixes.
    return Result<int64_t>::Err(ErrorCode::kProtocolError,
                                "TODO(Task 2): RespParser::ParseInt");
}

ParseResult RespParser::ParseInternal([[maybe_unused]] std::string_view input, [[maybe_unused]] size_t& offset, [[maybe_unused]] size_t depth) {
    // TODO(Task 2): parse one RESP value from the given buffer and update offset.
    return ParseResult::NeedMoreData();
}

ParseResult RespParser::Parse(std::string_view input) {
    // TODO(Task 2): append new bytes, parse one complete value, retain leftovers.
    input_buffer_.append(input);
    if (input_buffer_.empty()) return ParseResult::NeedMoreData();

    const std::string data = input_buffer_;
    const size_t consumed = input_buffer_.size();
    input_buffer_.clear();

    const char prefix = data[0];
    switch (prefix) {
        case '+': return ParseResult::Complete(consumed, RespValue::SimpleString(""));
        case '-': return ParseResult::Complete(consumed, RespValue::Error(""));
        case ':': return ParseResult::Complete(consumed, RespValue::Integer(0));
        case '$':
            if (data.rfind("$-1", 0) == 0) return ParseResult::Complete(consumed, RespValue::NullBulkString());
            return ParseResult::Complete(consumed, RespValue::BulkString(""));
        case '*': {
            if (data.rfind("*-1", 0) == 0) return ParseResult::Complete(consumed, RespValue::NullArray());
            if (data.find("$4\r\nPING") != std::string::npos) {
                return ParseResult::Complete(consumed, RespValue::MakeArray(PlaceholderCommand("PING", 1)));
            }
            if (data.find("$3\r\nSET") != std::string::npos) {
                return ParseResult::Complete(consumed, RespValue::MakeArray(PlaceholderCommand("SET", 3)));
            }
            if (data.find("$4\r\nINCR") != std::string::npos) {
                return ParseResult::Complete(consumed, RespValue::MakeArray(PlaceholderCommand("INCR", 2)));
            }
            if (data.find("$3\r\nGET") != std::string::npos) {
                return ParseResult::Complete(consumed, RespValue::MakeArray(PlaceholderCommand("GET", 2)));
            }
            if (data.rfind("*0", 0) == 0) return ParseResult::Complete(consumed, RespValue::MakeArray({}));
            if (data.rfind("*2", 0) == 0) return ParseResult::Complete(consumed, RespValue::MakeArray(PlaceholderArray(2)));
            return ParseResult::Complete(consumed, RespValue::MakeArray(PlaceholderArray(1)));
        }
        default:
            return ParseResult::ProtocolError(
                Error(ErrorCode::kProtocolError, "TODO(Task 2): parser not implemented"));
    }
}

}  // namespace miniredis
