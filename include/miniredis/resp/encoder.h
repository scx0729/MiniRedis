#pragma once

#include "value.h"
#include <optional>
#include <string>

namespace miniredis {

// Encodes RESP2 values to wire format
class RespEncoder {
public:
    // Encode a single RESP value to bytes
    static std::string Encode(const RespValue& value);

    // Encode an optional RESP value (null optional = Null Bulk String)
    static std::string Encode(const std::optional<RespValue>& value);

private:
    static std::string EncodeSimpleString(const std::string& s);
    static std::string EncodeError(const std::string& s);
    static std::string EncodeInteger(int64_t val);
    static std::string EncodeBulkString(const std::string& s);
    static std::string EncodeNullBulkString();
    static std::string EncodeArray(const RespValue::ArrayT& arr);
    static std::string EncodeNullArray();
};

}  // namespace miniredis
