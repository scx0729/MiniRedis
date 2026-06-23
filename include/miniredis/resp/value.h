#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace miniredis {

// RESP2 value types
enum class RespType : uint8_t {
    kSimpleString,
    kError,
    kInteger,
    kBulkString,
    kArray,
};

// A RESP2 value. Null states are represented by optional wrappers.
class RespValue {
public:
    using ArrayT = std::vector<std::optional<RespValue>>;

    // Simple String
    static RespValue SimpleString(std::string s) {
        RespValue v;
        v.type_ = RespType::kSimpleString;
        v.data_ = std::move(s);
        return v;
    }

    // Error
    static RespValue Error(std::string s) {
        RespValue v;
        v.type_ = RespType::kError;
        v.data_ = std::move(s);
        return v;
    }

    // Integer
    static RespValue Integer(int64_t val) {
        RespValue v;
        v.type_ = RespType::kInteger;
        v.data_ = val;
        return v;
    }

    // Bulk String (non-null)
    static RespValue BulkString(std::string s) {
        RespValue v;
        v.type_ = RespType::kBulkString;
        v.data_ = std::move(s);
        return v;
    }

    // Null Bulk String
    static RespValue NullBulkString() {
        RespValue v;
        v.type_ = RespType::kBulkString;
        v.data_ = std::monostate{};
        return v;
    }

    // Array (non-null)
    static RespValue MakeArray(ArrayT arr) {
        RespValue v;
        v.type_ = RespType::kArray;
        v.data_ = std::move(arr);
        return v;
    }

    // Null Array
    static RespValue NullArray() {
        RespValue v;
        v.type_ = RespType::kArray;
        v.data_ = std::monostate{};
        return v;
    }

    RespType type() const { return type_; }

    bool is_null() const { return std::holds_alternative<std::monostate>(data_); }

    const std::string& simple_string() const { return std::get<std::string>(data_); }
    const std::string& error_string() const { return std::get<std::string>(data_); }
    int64_t integer() const { return std::get<int64_t>(data_); }
    const std::string& bulk_string() const { return std::get<std::string>(data_); }
    const ArrayT& array() const { return std::get<ArrayT>(data_); }

private:
    RespType type_;
    std::variant<std::monostate, std::string, int64_t, ArrayT> data_;
};

}  // namespace miniredis
