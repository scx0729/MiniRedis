// TODO(Task 7): implement active expiration metadata.
#include "miniredis/storage/expiration.h"
#include "miniredis/storage/database.h"

namespace miniredis {

Expiration::Expiration(Clock& clock) : clock_(clock) {}

void Expiration::Add([[maybe_unused]] const std::string& key, [[maybe_unused]] int64_t expire_at_ms) {
    // TODO(Task 7): push/update expiration heap entry.
}

void Expiration::Remove([[maybe_unused]] const std::string& key) {
    // TODO(Task 7): mark a key as no longer actively tracked.
}

size_t Expiration::ActiveExpire([[maybe_unused]] Database& db, [[maybe_unused]] size_t max_sample) {
    // TODO(Task 7): expire at most max_sample currently expired keys.
    return 0;
}

bool Expiration::HasEntry([[maybe_unused]] const std::string& key) const {
    // TODO(Task 7): report whether a key has live expiration metadata.
    return false;
}

}  // namespace miniredis
