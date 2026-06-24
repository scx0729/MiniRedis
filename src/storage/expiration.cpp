#include "miniredis/storage/expiration.h"
#include "miniredis/storage/database.h"

namespace miniredis {

Expiration::Expiration(Clock& clock) : clock_(clock) {}

void Expiration::Add(const std::string& key, int64_t expire_at_ms) {
    heap_.push({expire_at_ms, key});
    active_expirations_[key] = expire_at_ms;
}

void Expiration::Remove(const std::string& key) {
    active_expirations_.erase(key);
    // Stale heap entries are discarded lazily during ActiveExpire.
}

bool Expiration::HasEntry(const std::string& key) const {
    return active_expirations_.count(key) > 0;
}

size_t Expiration::ActiveExpire(Database& db, size_t max_sample) {
    int64_t now = clock_.NowMs();
    size_t expired = 0;

    while (!heap_.empty() && expired < max_sample) {
        auto top = heap_.top();

        // Remove stale entries from deleted keys or overwritten TTLs.
        auto active_it = active_expirations_.find(top.key);
        if (active_it == active_expirations_.end() ||
            active_it->second != top.expire_at_ms) {
            heap_.pop();
            continue;
        }

        // If the earliest current entry is not expired, later entries are not either.
        if (top.expire_at_ms > now) {
            break;
        }

        heap_.pop();

        // Check the database as the source of truth before deleting.
        if (db.IsExpired(top.key)) {
            db.Del(top.key);
            ++expired;
        } else {
            active_expirations_.erase(top.key);
        }
    }

    return expired;
}

}  // namespace miniredis
