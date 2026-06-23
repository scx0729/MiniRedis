#include "miniredis/storage/expiration.h"
#include "miniredis/storage/database.h"

namespace miniredis {

Expiration::Expiration(Clock& clock) : clock_(clock) {}

void Expiration::Add(const std::string& key, int64_t expire_at_ms) {
    heap_.push({expire_at_ms, key});
    active_keys_.insert(key);
}

void Expiration::Remove(const std::string& key) {
    active_keys_.erase(key);
    // Can't efficiently remove from heap; lazy cleanup during ActiveExpire
}

bool Expiration::HasEntry(const std::string& key) const {
    return active_keys_.count(key) > 0;
}

size_t Expiration::ActiveExpire(Database& db, size_t max_sample) {
    int64_t now = clock_.NowMs();
    size_t expired = 0;

    while (!heap_.empty() && expired < max_sample) {
        auto top = heap_.top();

        // Remove stale entries (key was re-set with different TTL or deleted)
        if (active_keys_.count(top.key) == 0) {
            heap_.pop();
            continue;
        }

        // If not expired, stop
        if (top.expire_at_ms > now) {
            break;
        }

        heap_.pop();
        active_keys_.erase(top.key);

        // Check if key still exists and is actually expired
        if (db.IsExpired(top.key)) {
            db.Del(top.key);
            ++expired;
        }
    }

    return expired;
}

}  // namespace miniredis
