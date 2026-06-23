#pragma once

#include "value.h"
#include "miniredis/base/clock.h"
#include "miniredis/base/result.h"
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace miniredis {

class Expiration;

struct Entry {
    StorageValue value;
    int64_t expire_at_ms = -1;  // -1 means no expiry

    bool IsExpired(int64_t now_ms) const {
        return expire_at_ms > 0 && now_ms >= expire_at_ms;
    }
};

class Database {
public:
    explicit Database(Clock& clock, Expiration* expiration = nullptr);

    // Generic key operations
    bool Exists(const std::string& key);
    int64_t Del(const std::string& key);
    int64_t Del(const std::vector<std::string>& keys);
    Result<std::string> Type(const std::string& key);

    // String operations
    Result<void> Set(const std::string& key, std::string value,
                     int64_t expire_ms = -1, bool nx = false, bool xx = false);
    Result<std::optional<std::string>> Get(const std::string& key);
    Result<void> MSet(const std::vector<std::pair<std::string, std::string>>& kvs);
    Result<std::vector<std::optional<std::string>>> MGet(const std::vector<std::string>& keys);
    Result<int64_t> Append(const std::string& key, const std::string& value);
    Result<int64_t> Strlen(const std::string& key);
    Result<int64_t> Incr(const std::string& key);
    Result<int64_t> Decr(const std::string& key);

    // List operations
    Result<int64_t> LPush(const std::string& key, const std::vector<std::string>& elements);
    Result<int64_t> RPush(const std::string& key, const std::vector<std::string>& elements);
    Result<std::optional<std::string>> LPop(const std::string& key);
    Result<std::optional<std::string>> RPop(const std::string& key);
    Result<int64_t> LLen(const std::string& key);
    Result<std::vector<std::string>> LRange(const std::string& key, int64_t start, int64_t stop);

    // Hash operations
    Result<int64_t> HSet(const std::string& key,
                         const std::vector<std::pair<std::string, std::string>>& fvs);
    Result<std::optional<std::string>> HGet(const std::string& key, const std::string& field);
    Result<int64_t> HDel(const std::string& key, const std::vector<std::string>& fields);
    Result<int64_t> HLen(const std::string& key);
    Result<std::vector<std::string>> HGetAll(const std::string& key);

    // Expiration
    void SetExpireMs(const std::string& key, int64_t expire_ms);
    void ClearExpire(const std::string& key);
    int64_t GetExpireMs(const std::string& key) const;
    bool IsExpired(const std::string& key);

    // For active expiration
    size_t ExpireSample(size_t count);

    // Access raw entries (for testing)
    size_t Size() const { return data_.size(); }

private:
    // Returns entry if exists and not expired. Performs lazy expiration.
    Entry* FindEntry(const std::string& key);
    const Entry* FindEntry(const std::string& key) const;

    // Remove key from database
    void RemoveKey(const std::string& key);

    // Get or create string value, checking type
    Result<StringValue*> GetStringValue(const std::string& key);
    Result<ListValue*> GetListValue(const std::string& key);
    Result<HashValue*> GetHashValue(const std::string& key);

    // Check if key holds the expected type. Returns error if wrong type.
    Result<void> CheckType(const std::string& key, ValueType expected);

    std::unordered_map<std::string, Entry> data_;
    Clock& clock_;
    Expiration* expiration_;  // optional, not owned
};

}  // namespace miniredis
