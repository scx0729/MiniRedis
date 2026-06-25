// TODO(Task 2): implement an incremental RESP2 parser.
#include "miniredis/resp/parser.h"
#include "miniredis/base/error.h"
#include "miniredis/base/result.h"
#include "miniredis/resp/value.h"
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace miniredis {

RespParser::RespParser(ParserLimits limits) : limits_(limits) {}

void RespParser::Reset() {
    input_buffer_.clear();
}

bool RespParser::HasBufferedData() const {
    return !input_buffer_.empty();
}

size_t RespParser::FindCRLF([[maybe_unused]] std::string_view input, [[maybe_unused]] size_t offset) const {
    while(offset + 1 < input.size()){
        if(input.at(offset) == '\r' && input.at(offset + 1) == '\n'){
            return offset;
        }
        offset++;
    }
    return std::string_view::npos;
}

Result<int64_t> RespParser::ParseInt([[maybe_unused]] std::string_view text) const {
    if(text.empty()){
        return Result<int64_t>::Err(ErrorCode::kProtocolError, "empty integer");
    }
    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), val);
    if(ec != std::errc() || ptr != (text.data() + text.size())){
        return Result<int64_t>::Err(ErrorCode::kProtocolError, "invalid integer");
    }
    return Result<int64_t>::Ok(val);
}

ParseResult RespParser::ParseInternal([[maybe_unused]] std::string_view input, [[maybe_unused]] size_t& offset, [[maybe_unused]] size_t depth) {
    if(depth > limits_.max_depth){
        return ParseResult::ProtocolError(Error(ErrorCode::kMaxDepthExceeded, "max nesting depth exceeded"));
    }
    if(offset >= input.size()){
        return ParseResult::NeedMoreData();
    }
    auto prefix = input.at(offset);
    auto crlf = FindCRLF(input, offset);
    if(crlf == std::string_view::npos){
        return ParseResult::NeedMoreData();
    }
    auto context = input.substr(offset + 1, crlf - offset - 1);
    switch (prefix) {
        case '+':{
            auto consumed = crlf - offset + 2;
            offset = crlf + 2;
            return ParseResult::Complete(consumed, RespValue::SimpleString(std::string(context)));
        }
        case '-':{
            auto consumed = crlf - offset + 2;
            offset = crlf + 2;
            return ParseResult::Complete(consumed, RespValue::Error(std::string(context)));
        }
        case ':':{
            auto consumed = crlf - offset + 2;
            offset = crlf + 2;
            auto val = ParseInt(context);
            if(val.ok()){
                return ParseResult::Complete(consumed, RespValue::Integer(val.value()));
            } else {
                return ParseResult::ProtocolError(val.error());
            }
        }
        case '$':{
            auto len = ParseInt(context);
            if(!len.ok()){
                return ParseResult::ProtocolError(len.error());
            }
            if(len.value() < -1){
                return ParseResult::ProtocolError(Error(ErrorCode::kInvalidBulkLength, "invalid bulk string length"));
            }
            if(len.value() == -1){
                auto consumed = crlf - offset + 2;
                offset = crlf + 2;
                return ParseResult::Complete(consumed, RespValue::NullBulkString());
            }
            if(static_cast<size_t>(len.value()) > limits_.max_bulk_length){
                return ParseResult::ProtocolError(Error(ErrorCode::kInvalidBulkLength, "bulk string too long"));
            }
            auto bulk_len = static_cast<size_t>(len.value());
            auto data_start = crlf + 2;
            auto available = input.size() - data_start;
            if(available < bulk_len + 2){
                return ParseResult::NeedMoreData();
            }
            auto consumed = data_start + bulk_len + 2 - offset;
            context = input.substr(data_start, bulk_len);
            offset = data_start + bulk_len + 2;
            return ParseResult::Complete(consumed, RespValue::BulkString(std::string(context)));
        }
        case '*':{
            auto count = ParseInt(context);
            if(!count.ok()){
                return ParseResult::ProtocolError(count.error());
            }
            if(count.value() < -1){
                return ParseResult::ProtocolError(Error(ErrorCode::kInvalidArrayLength, "invalid array length"));
            }
            if(count.value() == -1){
                auto consumed = crlf - offset + 2;
                offset = crlf + 2;
                return ParseResult::Complete(consumed, RespValue::NullArray());
            }
            if(static_cast<size_t>(count.value()) > limits_.max_array_length){
                return ParseResult::ProtocolError(Error(ErrorCode::kInvalidArrayLength, "array too long"));
            }
            auto array_start = offset;
            offset = crlf + 2;
            RespValue::ArrayT arr;
            arr.reserve(static_cast<size_t>(count.value()));
            for(size_t i = 0;i < static_cast<size_t>(count.value());i++){
                auto elem_result = ParseInternal(input, offset, depth + 1);
                if(elem_result.status == ParseStatus::kNeedMoreData){
                    offset = array_start;
                    return elem_result;
                }
                if(elem_result.status == ParseStatus::kProtocolError){
                    offset = array_start;
                    return elem_result;
                }
                arr.emplace_back(elem_result.value);
            }
            auto consumed = offset - array_start;
            return ParseResult::Complete(consumed, RespValue::MakeArray(arr));
        }
    }
    return ParseResult::ProtocolError(Error(ErrorCode::kProtocolError,  "unknown RESP type prefix"));
}

ParseResult RespParser::Parse(std::string_view input) {
    input_buffer_.append(input);
    if (input_buffer_.empty()) return ParseResult::NeedMoreData();
    size_t offset = 0;
    auto result = ParseInternal(input_buffer_, offset, 0);
    if(result.status == ParseStatus::kProtocolError){
        input_buffer_.clear();
    }
    if(result.status == ParseStatus::kComplete){
        input_buffer_.erase(0, result.consumed);
    }
    return result;
}

}  // namespace miniredis
