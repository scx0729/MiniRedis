// TODO(Task 1): implement RESP2 encoding.
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/value.h"
#include <string>

namespace miniredis {

std::string RespEncoder::Encode([[maybe_unused]] const RespValue& value) {
    switch (value.type()) {
        case RespType::kSimpleString:
            return EncodeSimpleString(value.simple_string());
        case RespType::kError:
            return EncodeError(value.error_string());
        case RespType::kInteger:
            return EncodeInteger(value.integer());
        case RespType::kBulkString:
            if(value.is_null()) return EncodeNullBulkString();
            return EncodeBulkString(value.bulk_string());
        case RespType::kArray:
            if(value.is_null()) return EncodeNullArray();
            return EncodeArray(value.array());
    }
    return {};
}

std::string RespEncoder::Encode(const std::optional<RespValue>& value) {
    if (!value.has_value()) return EncodeNullBulkString();
    return Encode(*value);
}

std::string RespEncoder::EncodeSimpleString([[maybe_unused]] const std::string& s) {
    return "+" + s + "\r\n";
}

std::string RespEncoder::EncodeError([[maybe_unused]] const std::string& s) {
    return "-" + s + "\r\n";
}

std::string RespEncoder::EncodeInteger([[maybe_unused]] int64_t val) {
    return ":" + std::to_string(val) + "\r\n";
}

std::string RespEncoder::EncodeBulkString([[maybe_unused]] const std::string& s) {
    return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n";
}

std::string RespEncoder::EncodeNullBulkString() {
    // This helper is intentionally provided because optional<T> uses it too.
    return "$-1\r\n";
}

std::string RespEncoder::EncodeArray([[maybe_unused]] const RespValue::ArrayT& arr) {
    std::string result = "*" + std::to_string(arr.size()) + "\r\n";
    for(auto &value : arr){
        if(value.has_value()){
            result += Encode(value);
        } else {
            result += EncodeNullBulkString();
        }
    }
    return result;
}

std::string RespEncoder::EncodeNullArray() {
    return "*-1\r\n";
}

}  // namespace miniredis
