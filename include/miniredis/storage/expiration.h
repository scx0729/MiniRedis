#pragma once

#include "miniredis/base/clock.h"
#include <cstdint>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

namespace miniredis {

class Database;

// Active expiration manager
// Uses a min-heap of (expire_at_ms, key) pairs
class Expiration {
public:
    explicit Expiration(Clock& clock);

    // Register a key with its expiration time
    void Add(const std::string& key, int64_t expire_at_ms);

    // Remove a key from expiration tracking
    void Remove(const std::string& key);

    // Perform active expiration sweep. Returns number of keys expired.
    size_t ActiveExpire(Database& db, size_t max_sample);

    // Check if a key has a pending expiration entry
    bool HasEntry(const std::string& key) const;

private:
    struct ExpireEntry {
        int64_t expire_at_ms;
        std::string key;

        // For min-heap (earliest expiry first)
        bool operator>(const ExpireEntry& other) const {
            return expire_at_ms > other.expire_at_ms;
        }
    };

    Clock& clock_;
    std::priority_queue<ExpireEntry, std::vector<ExpireEntry>, std::greater<>> heap_;
    std::unordered_set<std::string> active_keys_;  // tracks which keys are in heap
};

}  // namespace miniredis
