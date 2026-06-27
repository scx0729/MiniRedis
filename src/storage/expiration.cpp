// TODO(Task 7): implement active expiration metadata.
#include "miniredis/storage/expiration.h"
#include "miniredis/storage/database.h"
#include <cstddef>

namespace miniredis {

Expiration::Expiration(Clock& clock) : clock_(clock) {}

void Expiration::Add([[maybe_unused]] const std::string& key, [[maybe_unused]] int64_t expire_at_ms) {
    heap_.push({expire_at_ms, key});
    active_expirations_.emplace(key, expire_at_ms);
}

void Expiration::Remove([[maybe_unused]] const std::string& key) {
    active_expirations_.erase(key);
}

size_t Expiration::ActiveExpire([[maybe_unused]] Database& db, [[maybe_unused]] size_t max_sample) {
    size_t count = 0;    
    while(!heap_.empty() && count < max_sample){
        auto top = heap_.top();
        auto it = active_expirations_.find(top.key);
        if(it == active_expirations_.end() || it->second != top.expire_at_ms) {
            heap_.pop();
            continue;
        }
        if(top.expire_at_ms > clock_.NowMs()) { break; }
        heap_.pop();
        if(db.IsExpired(top.key)){
            db.Del(top.key);
            count++;
        } else {
            Remove(top.key);
        }
        
    }
    return count;
}

bool Expiration::HasEntry([[maybe_unused]] const std::string& key) const {
    if(active_expirations_.find(key) == active_expirations_.end()) return false;
    return true;
}

}  // namespace miniredis
