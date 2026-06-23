#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include <algorithm>
#include <charconv>
#include <cerrno>
#include <cstdlib>
#include <limits>

namespace miniredis {

Database::Database(Clock& clock, Expiration* expiration)
    : clock_(clock), expiration_(expiration) {}

Entry* Database::FindEntry(const std::string& key) {
    auto it = data_.find(key);
    if (it == data_.end()) return nullptr;

    // Lazy expiration check
    if (it->second.IsExpired(clock_.NowMs())) {
        if (expiration_) expiration_->Remove(key);
        data_.erase(it);
        return nullptr;
    }
    return &it->second;
}

const Entry* Database::FindEntry(const std::string& key) const {
    auto it = data_.find(key);
    if (it == data_.end()) return nullptr;

    if (it->second.IsExpired(clock_.NowMs())) {
        // Can't erase in const method; caller should use non-const version
        return nullptr;
    }
    return &it->second;
}

void Database::RemoveKey(const std::string& key) {
    if (expiration_) expiration_->Remove(key);
    data_.erase(key);
}

Result<void> Database::CheckType(const std::string& key, ValueType expected) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<void>::Ok();  // key doesn't exist, OK

    ValueType actual;
    if (std::holds_alternative<StringValue>(entry->value)) {
        actual = ValueType::kString;
    } else if (std::holds_alternative<ListValue>(entry->value)) {
        actual = ValueType::kList;
    } else {
        actual = ValueType::kHash;
    }

    if (actual != expected) {
        return Result<void>::Err(ErrorCode::kWrongType);
    }
    return Result<void>::Ok();
}

Result<StringValue*> Database::GetStringValue(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) {
        // Create new string value
        auto [it, _] = data_.emplace(key, Entry{StringValue{""}, -1});
        return &std::get<StringValue>(it->second.value);
    }
    if (!std::holds_alternative<StringValue>(entry->value)) {
        return Result<StringValue*>::Err(ErrorCode::kWrongType);
    }
    return &std::get<StringValue>(entry->value);
}

Result<ListValue*> Database::GetListValue(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) {
        auto [it, _] = data_.emplace(key, Entry{ListValue{{}}, -1});
        return &std::get<ListValue>(it->second.value);
    }
    if (!std::holds_alternative<ListValue>(entry->value)) {
        return Result<ListValue*>::Err(ErrorCode::kWrongType);
    }
    return &std::get<ListValue>(entry->value);
}

Result<HashValue*> Database::GetHashValue(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) {
        auto [it, _] = data_.emplace(key, Entry{HashValue{{}}, -1});
        return &std::get<HashValue>(it->second.value);
    }
    if (!std::holds_alternative<HashValue>(entry->value)) {
        return Result<HashValue*>::Err(ErrorCode::kWrongType);
    }
    return &std::get<HashValue>(entry->value);
}

// Generic key operations

bool Database::Exists(const std::string& key) {
    return FindEntry(key) != nullptr;
}

int64_t Database::Del(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return 0;
    RemoveKey(key);
    return 1;
}

int64_t Database::Del(const std::vector<std::string>& keys) {
    int64_t count = 0;
    for (const auto& key : keys) {
        if (FindEntry(key)) {
            RemoveKey(key);
            ++count;
        }
    }
    return count;
}

Result<std::string> Database::Type(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::string>::Ok("none");

    if (std::holds_alternative<StringValue>(entry->value))
        return Result<std::string>::Ok("string");
    if (std::holds_alternative<ListValue>(entry->value))
        return Result<std::string>::Ok("list");
    return Result<std::string>::Ok("hash");
}

// String operations

Result<void> Database::Set(const std::string& key, std::string value,
                           int64_t expire_ms, bool nx, bool xx) {
    auto* entry = FindEntry(key);
    bool exists = (entry != nullptr);

    // NX: only set if not exists
    if (nx && exists) return Result<void>::Ok();
    // XX: only set if exists
    if (xx && !exists) return Result<void>::Ok();

    // Check type if key exists
    if (exists && !std::holds_alternative<StringValue>(entry->value)) {
        // SET overwrites any type
        RemoveKey(key);
    }

    // Set value
    if (!exists || !std::holds_alternative<StringValue>(data_[key].value)) {
        data_[key] = Entry{StringValue{std::move(value)}, -1};
    } else {
        std::get<StringValue>(data_[key].value).data = std::move(value);
    }

    // Handle TTL
    if (expire_ms > 0) {
        int64_t expire_at = clock_.NowMs() + expire_ms;
        data_[key].expire_at_ms = expire_at;
        if (expiration_) expiration_->Add(key, expire_at);
    } else if (expire_ms == -1) {
        // No TTL specified - clear existing TTL
        data_[key].expire_at_ms = -1;
        if (expiration_) expiration_->Remove(key);
    }
    // expire_ms == 0 means we're not changing TTL (used by INCR etc.)

    return Result<void>::Ok();
}

Result<std::optional<std::string>> Database::Get(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::optional<std::string>>::Ok(std::nullopt);

    if (!std::holds_alternative<StringValue>(entry->value)) {
        return Result<std::optional<std::string>>::Err(ErrorCode::kWrongType);
    }

    return Result<std::optional<std::string>>::Ok(
        std::get<StringValue>(entry->value).data);
}

Result<void> Database::MSet(const std::vector<std::pair<std::string, std::string>>& kvs) {
    for (const auto& [key, value] : kvs) {
        auto result = Set(key, value);
        if (!result.ok()) return result;
    }
    return Result<void>::Ok();
}

Result<std::vector<std::optional<std::string>>> Database::MGet(
    const std::vector<std::string>& keys) {
    std::vector<std::optional<std::string>> results;
    results.reserve(keys.size());

    for (const auto& key : keys) {
        auto result = Get(key);
        if (!result.ok()) {
            // WrongType: return nil for this key (like Redis)
            results.push_back(std::nullopt);
        } else {
            results.push_back(std::move(result.value()));
        }
    }
    return Result<std::vector<std::optional<std::string>>>::Ok(std::move(results));
}

Result<int64_t> Database::Append(const std::string& key, const std::string& value) {
    auto* entry = FindEntry(key);
    if (!entry) {
        // Create new key
        auto result = Set(key, value);
        if (!result.ok()) return Result<int64_t>::Err(result.error_code());
        return Result<int64_t>::Ok(static_cast<int64_t>(value.size()));
    }

    if (!std::holds_alternative<StringValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    auto& str = std::get<StringValue>(entry->value).data;
    str.append(value);
    return Result<int64_t>::Ok(static_cast<int64_t>(str.size()));
}

Result<int64_t> Database::Strlen(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<int64_t>::Ok(0);

    if (!std::holds_alternative<StringValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    return Result<int64_t>::Ok(
        static_cast<int64_t>(std::get<StringValue>(entry->value).data.size()));
}

Result<int64_t> Database::Incr(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) {
        // Create with value "0", then increment
        auto result = Set(key, "1");
        if (!result.ok()) return Result<int64_t>::Err(result.error_code());
        return Result<int64_t>::Ok(1);
    }

    if (!std::holds_alternative<StringValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    auto& str = std::get<StringValue>(entry->value).data;

    // Parse as int64
    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
    if (ec != std::errc() || ptr != str.data() + str.size()) {
        return Result<int64_t>::Err(ErrorCode::kNotInteger);
    }

    // Check overflow
    if (val > INT64_MAX - 1) {
        return Result<int64_t>::Err(ErrorCode::kOverflow);
    }

    val += 1;
    str = std::to_string(val);

    return Result<int64_t>::Ok(val);
}

Result<int64_t> Database::Decr(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) {
        auto result = Set(key, "-1");
        if (!result.ok()) return Result<int64_t>::Err(result.error_code());
        return Result<int64_t>::Ok(-1);
    }

    if (!std::holds_alternative<StringValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    auto& str = std::get<StringValue>(entry->value).data;

    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
    if (ec != std::errc() || ptr != str.data() + str.size()) {
        return Result<int64_t>::Err(ErrorCode::kNotInteger);
    }

    if (val < INT64_MIN + 1) {
        return Result<int64_t>::Err(ErrorCode::kOverflow);
    }

    val -= 1;
    str = std::to_string(val);

    return Result<int64_t>::Ok(val);
}

// List operations

Result<int64_t> Database::LPush(const std::string& key,
                                const std::vector<std::string>& elements) {
    auto result = GetListValue(key);
    if (!result.ok()) return Result<int64_t>::Err(result.error_code());

    auto& list = result.value()->elements;
    for (const auto& elem : elements) {
        list.push_front(elem);
    }

    return Result<int64_t>::Ok(static_cast<int64_t>(list.size()));
}

Result<int64_t> Database::RPush(const std::string& key,
                                const std::vector<std::string>& elements) {
    auto result = GetListValue(key);
    if (!result.ok()) return Result<int64_t>::Err(result.error_code());

    auto& list = result.value()->elements;
    for (const auto& elem : elements) {
        list.push_back(elem);
    }

    return Result<int64_t>::Ok(static_cast<int64_t>(list.size()));
}

Result<std::optional<std::string>> Database::LPop(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::optional<std::string>>::Ok(std::nullopt);

    if (!std::holds_alternative<ListValue>(entry->value)) {
        return Result<std::optional<std::string>>::Err(ErrorCode::kWrongType);
    }

    auto& list = std::get<ListValue>(entry->value).elements;
    if (list.empty()) return Result<std::optional<std::string>>::Ok(std::nullopt);

    std::string value = std::move(list.front());
    list.pop_front();

    // Remove key if list is now empty
    if (list.empty()) RemoveKey(key);

    return Result<std::optional<std::string>>::Ok(std::move(value));
}

Result<std::optional<std::string>> Database::RPop(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::optional<std::string>>::Ok(std::nullopt);

    if (!std::holds_alternative<ListValue>(entry->value)) {
        return Result<std::optional<std::string>>::Err(ErrorCode::kWrongType);
    }

    auto& list = std::get<ListValue>(entry->value).elements;
    if (list.empty()) return Result<std::optional<std::string>>::Ok(std::nullopt);

    std::string value = std::move(list.back());
    list.pop_back();

    if (list.empty()) RemoveKey(key);

    return Result<std::optional<std::string>>::Ok(std::move(value));
}

Result<int64_t> Database::LLen(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<int64_t>::Ok(0);

    if (!std::holds_alternative<ListValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    return Result<int64_t>::Ok(
        static_cast<int64_t>(std::get<ListValue>(entry->value).elements.size()));
}

Result<std::vector<std::string>> Database::LRange(const std::string& key,
                                                   int64_t start, int64_t stop) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::vector<std::string>>::Ok({});

    if (!std::holds_alternative<ListValue>(entry->value)) {
        return Result<std::vector<std::string>>::Err(ErrorCode::kWrongType);
    }

    const auto& list = std::get<ListValue>(entry->value).elements;
    int64_t len = static_cast<int64_t>(list.size());

    // Normalize negative indices
    if (start < 0) start += len;
    if (stop < 0) stop += len;

    // Clamp
    if (start < 0) start = 0;
    if (stop >= len) stop = len - 1;

    std::vector<std::string> result;
    if (start > stop || start >= len) return Result<std::vector<std::string>>::Ok(std::move(result));

    result.reserve(static_cast<size_t>(stop - start + 1));
    for (int64_t i = start; i <= stop; ++i) {
        result.push_back(list[static_cast<size_t>(i)]);
    }

    return Result<std::vector<std::string>>::Ok(std::move(result));
}

// Hash operations

Result<int64_t> Database::HSet(const std::string& key,
                               const std::vector<std::pair<std::string, std::string>>& fvs) {
    auto result = GetHashValue(key);
    if (!result.ok()) return Result<int64_t>::Err(result.error_code());

    auto& fields = result.value()->fields;
    int64_t added = 0;

    for (const auto& [field, value] : fvs) {
        auto [_, inserted] = fields.insert_or_assign(field, value);
        if (inserted) ++added;
    }

    return Result<int64_t>::Ok(added);
}

Result<std::optional<std::string>> Database::HGet(const std::string& key,
                                                    const std::string& field) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::optional<std::string>>::Ok(std::nullopt);

    if (!std::holds_alternative<HashValue>(entry->value)) {
        return Result<std::optional<std::string>>::Err(ErrorCode::kWrongType);
    }

    const auto& fields = std::get<HashValue>(entry->value).fields;
    auto it = fields.find(field);
    if (it == fields.end()) return Result<std::optional<std::string>>::Ok(std::nullopt);

    return Result<std::optional<std::string>>::Ok(it->second);
}

Result<int64_t> Database::HDel(const std::string& key,
                               const std::vector<std::string>& fields) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<int64_t>::Ok(0);

    if (!std::holds_alternative<HashValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    auto& hash_fields = std::get<HashValue>(entry->value).fields;
    int64_t removed = 0;

    for (const auto& field : fields) {
        removed += static_cast<int64_t>(hash_fields.erase(field));
    }

    // Remove key if hash is now empty
    if (hash_fields.empty()) RemoveKey(key);

    return Result<int64_t>::Ok(removed);
}

Result<int64_t> Database::HLen(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<int64_t>::Ok(0);

    if (!std::holds_alternative<HashValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }

    return Result<int64_t>::Ok(
        static_cast<int64_t>(std::get<HashValue>(entry->value).fields.size()));
}

Result<std::vector<std::string>> Database::HGetAll(const std::string& key) {
    auto* entry = FindEntry(key);
    if (!entry) return Result<std::vector<std::string>>::Ok({});

    if (!std::holds_alternative<HashValue>(entry->value)) {
        return Result<std::vector<std::string>>::Err(ErrorCode::kWrongType);
    }

    const auto& fields = std::get<HashValue>(entry->value).fields;
    std::vector<std::string> result;
    result.reserve(fields.size() * 2);

    // std::map is already sorted, so output is deterministic
    for (const auto& [field, value] : fields) {
        result.push_back(field);
        result.push_back(value);
    }

    return Result<std::vector<std::string>>::Ok(std::move(result));
}

// Expiration

void Database::SetExpireMs(const std::string& key, int64_t expire_ms) {
    auto* entry = FindEntry(key);
    if (!entry) return;

    int64_t expire_at = clock_.NowMs() + expire_ms;
    entry->expire_at_ms = expire_at;
    if (expiration_) expiration_->Add(key, expire_at);
}

void Database::ClearExpire(const std::string& key) {
    auto it = data_.find(key);
    if (it == data_.end()) return;
    it->second.expire_at_ms = -1;
    if (expiration_) expiration_->Remove(key);
}

int64_t Database::GetExpireMs(const std::string& key) const {
    auto it = data_.find(key);
    if (it == data_.end()) return -2;  // key not exists
    if (it->second.expire_at_ms < 0) return -1;  // no expiry
    int64_t remaining = it->second.expire_at_ms - clock_.NowMs();
    if (remaining <= 0) return -2;  // expired
    return remaining;
}

bool Database::IsExpired(const std::string& key) {
    auto it = data_.find(key);
    if (it == data_.end()) return false;
    return it->second.IsExpired(clock_.NowMs());
}

size_t Database::ExpireSample(size_t count) {
    if (!expiration_) return 0;
    return expiration_->ActiveExpire(*this, count);
}

}  // namespace miniredis
