// TODO(Task 1): implement RESP2 encoding.
#include "miniredis/resp/encoder.h"

namespace miniredis {

std::string RespEncoder::Encode(const RespValue&) {
    // TODO(Task 1): dispatch by RespValue::type().
    return "-ERR TODO(Task 1): RespEncoder::Encode\r\n";
}

std::string RespEncoder::Encode(const std::optional<RespValue>& value) {
    if (!value.has_value()) return EncodeNullBulkString();
    return Encode(*value);
}

std::string RespEncoder::EncodeSimpleString(const std::string&) {
    // TODO(Task 1): +<payload>\r\n
    return "";
}

std::string RespEncoder::EncodeError(const std::string&) {
    // TODO(Task 1): -<payload>\r\n
    return "";
}

std::string RespEncoder::EncodeInteger(int64_t) {
    // TODO(Task 1): :<number>\r\n
    return "";
}

std::string RespEncoder::EncodeBulkString(const std::string&) {
    // TODO(Task 1): $<byte length>\r\n<payload>\r\n
    return "";
}

std::string RespEncoder::EncodeNullBulkString() {
    // This helper is intentionally provided because optional<T> uses it too.
    return "$-1\r\n";
}

std::string RespEncoder::EncodeArray(const RespValue::ArrayT&) {
    // TODO(Task 1): encode each array element recursively.
    return "";
}

std::string RespEncoder::EncodeNullArray() {
    return "*-1\r\n";
}

}  // namespace miniredis
