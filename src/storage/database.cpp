// TODO(Task 3, Task 4, Task 7): implement the typed in-memory database.
#include "miniredis/storage/database.h"
#include "miniredis/base/error.h"
#include "miniredis/base/result.h"
#include "miniredis/storage/expiration.h"
#include "miniredis/storage/value.h"
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace miniredis {

Database::Database(Clock& clock, Expiration* expiration)
    : clock_(clock), expiration_(expiration) {}

Entry* Database::FindEntry([[maybe_unused]] const std::string& key) {
    auto it = data_.find(key);
    if(it == data_.end()) { return nullptr; }
    return &it->second;
}

const Entry* Database::FindEntry([[maybe_unused]] const std::string& key) const {
    auto it = data_.find(key);
    if(it == data_.end()) { return nullptr; }
    return &it->second;
}

void Database::RemoveKey([[maybe_unused]] const std::string& key) {
    data_.erase(key);
}

Result<void> Database::CheckType([[maybe_unused]] const std::string& key, [[maybe_unused]] ValueType expected) {
    auto entry = FindEntry(key);
    if(entry == nullptr) { return Result<void>::Ok(); }
    ValueType actual;
    if(std::holds_alternative<StringValue>(entry->value)){
        actual = ValueType::kString;
    } else if (std::holds_alternative<ListValue>(entry->value)) {
        actual = ValueType::kList;
    } else {
        actual = ValueType::kHash;
    }
    if(actual != expected){
        return Result<void>::Err(ErrorCode::kWrongType);
    }
    return Result<void>::Ok();
}

Result<StringValue*> Database::GetStringValue(const std::string& key) {
    auto [it, inserted] = data_.emplace(key, Entry{StringValue{{}}, -1});
    if(inserted) {  return Result<StringValue*>::Ok(&std::get<StringValue>(it->second.value)); }
    if(!CheckType(key, ValueType::kString).ok()){
        return Result<StringValue*>::Err(ErrorCode::kWrongType); 
    }
    return Result<StringValue*>::Ok(&std::get<StringValue>(it->second.value));
}

Result<ListValue*> Database::GetListValue(const std::string& key) {
    auto [it, inserted] = data_.emplace(key, Entry{ListValue{{}}, -1});
    if(inserted) {  return Result<ListValue*>::Ok(&std::get<ListValue>(it->second.value)); }
    if(!CheckType(key, ValueType::kList).ok()){
        return Result<ListValue*>::Err(ErrorCode::kWrongType); 
    }
    return Result<ListValue*>::Ok(&std::get<ListValue>(it->second.value));
}

Result<HashValue*> Database::GetHashValue(const std::string& key) {
    auto [it, inserted] = data_.emplace(key, Entry{HashValue{{}}, -1});
    if(inserted) {  return Result<HashValue*>::Ok(&std::get<HashValue>(it->second.value)); }
    if(!CheckType(key, ValueType::kHash).ok()){
        return Result<HashValue*>::Err(ErrorCode::kWrongType); 
    }
    return Result<HashValue*>::Ok(&std::get<HashValue>(it->second.value));
}

bool Database::Exists([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry != nullptr) { return true; }
    return false;
}

int64_t Database::Del([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry != nullptr){
        RemoveKey(key);
        return 1;
    }
    return 0;
}

int64_t Database::Del([[maybe_unused]] const std::vector<std::string>& keys) {
    int64_t count = 0;
    for(auto key : keys){
        auto entry = FindEntry(key);
        if(entry != nullptr){
            RemoveKey(key);
            count++;
        }
    }
    return count;
}

Result<std::string> Database::Type([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry == nullptr) { return Result<std::string>::Ok("none"); }
    if(CheckType(key, ValueType::kString).ok()){
        return Result<std::string>::Ok("string"); 
    } else if (CheckType(key, ValueType::kList).ok()) {
        return Result<std::string>::Ok("list"); 
    } else {
        return Result<std::string>::Ok("hash"); 
    }
}

Result<void> Database::Set([[maybe_unused]] const std::string& key, [[maybe_unused]] std::string value, [[maybe_unused]] int64_t expire_ms, [[maybe_unused]] bool nx, [[maybe_unused]] bool xx) {
    auto entry = FindEntry(key);
    auto exist = entry == nullptr ? false : true;
    if(nx && exist) { return Result<void>::Ok(); }
    if(xx && !exist) { return Result<void>::Ok(); }
    if(exist && !CheckType(key, ValueType::kString).ok()){
        RemoveKey(key);
    }
    data_[key] = Entry{StringValue{std::move(value)}, -1};
    return Result<void>::Ok();
}

Result<std::optional<std::string>> Database::Get([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry == nullptr) { return Result<std::optional<std::string>>::Ok(std::nullopt); }
    if(!CheckType(key, ValueType::kString).ok()) { return Result<std::optional<std::string>>::Err(ErrorCode::kWrongType); }
    return Result<std::optional<std::string>>::Ok(std::get<StringValue>(entry->value).data);
}

Result<void> Database::MSet([[maybe_unused]] const std::vector<std::pair<std::string, std::string>>& kvs) {
    for(auto &[key, value] : kvs){
        auto result = Set(key, value);
        if(!result.ok()) return result;
    }
    return Result<void>::Ok();
}

Result<std::vector<std::optional<std::string>>> Database::MGet(
    const std::vector<std::string>& keys) {
    std::vector<std::optional<std::string>> values;
    values.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i){
        auto result = Get(keys[i]);
        if(!result.ok()) { values.push_back(std::nullopt); }
        else { values.push_back(result.value()); }
    }
    return Result<std::vector<std::optional<std::string>>>::Ok(std::move(values));
}

Result<int64_t> Database::Append([[maybe_unused]] const std::string& key, [[maybe_unused]] const std::string& value) {
    auto entry = FindEntry(key);
    if(entry == nullptr){
        Set(key, value);
        return Result<int64_t>::Ok(static_cast<int64_t>(value.size()));
    }
    if(!CheckType(key, ValueType::kString).ok()){
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }
    auto &str = std::get<StringValue>(entry->value).data;
    str.append(value);
    return Result<int64_t>::Ok(static_cast<int64_t>(str.size()));
}

Result<int64_t> Database::Strlen([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry == nullptr){
        return Result<int64_t>::Ok(0);
    }
    if(!CheckType(key, ValueType::kString).ok()){
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }
    auto &str = std::get<StringValue>(entry->value).data;
    return Result<int64_t>::Ok(static_cast<int64_t>(str.size()));
}

Result<int64_t> Database::Incr([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry == nullptr){
        Set(key, "1");
        return Result<int64_t>::Ok(1);
    }
    if(!CheckType(key, ValueType::kString).ok()){
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }
    auto &str = std::get<StringValue>(entry->value).data;
    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
    if(ec != std::errc() || ptr != (str.data() + str.size())){
        return Result<int64_t>::Err(ErrorCode::kNotInteger);
    }
    if(val > INT64_MAX - 1){
        return Result<int64_t>::Err(ErrorCode::kOverflow);
    }
    val++;
    str = std::to_string(val);
    return Result<int64_t>::Ok(val);
}

Result<int64_t> Database::Decr([[maybe_unused]] const std::string& key) {
    auto entry = FindEntry(key);
    if(entry == nullptr){
        Set(key, "-1");
        return Result<int64_t>::Ok(-1);
    }
    if(!CheckType(key, ValueType::kString).ok()){
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }
    auto &str = std::get<StringValue>(entry->value).data;
    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
    if(ec != std::errc() || ptr != (str.data() + str.size())){
        return Result<int64_t>::Err(ErrorCode::kNotInteger);
    }
    if(val < INT64_MIN + 1){
        return Result<int64_t>::Err(ErrorCode::kOverflow);
    }
    val--;
    str = std::to_string(val);
    return Result<int64_t>::Ok(val);
}

Result<int64_t> Database::LPush([[maybe_unused]] const std::string& key, [[maybe_unused]] const std::vector<std::string>& elements) {
    // TODO(Task 4): push elements at the head of a list.
    return Result<int64_t>::Ok(0);
}

Result<int64_t> Database::RPush([[maybe_unused]] const std::string& key, [[maybe_unused]] const std::vector<std::string>& elements) {
    // TODO(Task 4): push elements at the tail of a list.
    return Result<int64_t>::Ok(0);
}

Result<std::optional<std::string>> Database::LPop([[maybe_unused]] const std::string& key) {
    // TODO(Task 4): pop the head element and delete empty lists.
    return Result<std::optional<std::string>>::Ok(std::string{});
}

Result<std::optional<std::string>> Database::RPop([[maybe_unused]] const std::string& key) {
    // TODO(Task 4): pop the tail element and delete empty lists.
    return Result<std::optional<std::string>>::Ok(std::string{});
}

Result<int64_t> Database::LLen([[maybe_unused]] const std::string& key) {
    // Task 3 already requires wrong-type detection for string keys.
    // Task 4 will complete the list length behavior.
    auto* entry = FindEntry(key);
    if (entry == nullptr) return Result<int64_t>::Ok(0);
    if (!std::holds_alternative<ListValue>(entry->value)) {
        return Result<int64_t>::Err(ErrorCode::kWrongType);
    }
    return Result<int64_t>::Ok(
        static_cast<int64_t>(std::get<ListValue>(entry->value).elements.size()));
}

Result<std::vector<std::string>> Database::LRange([[maybe_unused]] const std::string& key, [[maybe_unused]] int64_t start, [[maybe_unused]] int64_t stop) {
    // TODO(Task 4): implement Redis-style inclusive ranges with negative indexes.
    return Result<std::vector<std::string>>::Ok({});
}

Result<int64_t> Database::HSet(
    [[maybe_unused]] const std::string& key,
    [[maybe_unused]] const std::vector<std::pair<std::string, std::string>>& fvs) {
    // TODO(Task 4): set hash fields and return number of newly-added fields.
    return Result<int64_t>::Ok(0);
}

Result<std::optional<std::string>> Database::HGet([[maybe_unused]] const std::string& key, [[maybe_unused]] const std::string& field) {
    // TODO(Task 4): get one hash field or nil.
    return Result<std::optional<std::string>>::Ok(std::string{});
}

Result<int64_t> Database::HDel([[maybe_unused]] const std::string& key, [[maybe_unused]] const std::vector<std::string>& fields) {
    // TODO(Task 4): delete hash fields and delete empty hashes.
    return Result<int64_t>::Ok(0);
}

Result<int64_t> Database::HLen([[maybe_unused]] const std::string& key) {
    // TODO(Task 4): return hash field count.
    return Result<int64_t>::Ok(0);
}

Result<std::vector<std::string>> Database::HGetAll([[maybe_unused]] const std::string& key) {
    // TODO(Task 4): return deterministic field/value pairs.
    return Result<std::vector<std::string>>::Ok({});
}

void Database::SetExpireMs([[maybe_unused]] const std::string& key, [[maybe_unused]] int64_t expire_ms) {
    // TODO(Task 7): set/update absolute expiration metadata.
}

void Database::ClearExpire([[maybe_unused]] const std::string& key) {
    // TODO(Task 7): remove expiration metadata.
}

int64_t Database::GetExpireMs([[maybe_unused]] const std::string& key) const {
    // TODO(Task 7): return -2 missing, -1 persistent, or remaining ms.
    return -2;
}

bool Database::IsExpired([[maybe_unused]] const std::string& key) {
    // TODO(Task 7): check whether key is expired at current clock time.
    return false;
}

size_t Database::ExpireSample([[maybe_unused]] size_t count) {
    // TODO(Task 7): delegate active expiration to Expiration.
    return 0;
}

}  // namespace miniredis
