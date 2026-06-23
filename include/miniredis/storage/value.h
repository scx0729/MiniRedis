#pragma once

#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <variant>

namespace miniredis {

enum class ValueType : uint8_t {
    kString,
    kList,
    kHash,
};

struct StringValue {
    std::string data;
};

struct ListValue {
    std::deque<std::string> elements;
};

struct HashValue {
    // Using std::map for deterministic ordering (sorted by key)
    std::map<std::string, std::string> fields;
};

using StorageValue = std::variant<StringValue, ListValue, HashValue>;

inline const char* ValueTypeName(ValueType type) {
    switch (type) {
        case ValueType::kString: return "string";
        case ValueType::kList: return "list";
        case ValueType::kHash: return "hash";
    }
    return "none";
}

}  // namespace miniredis
