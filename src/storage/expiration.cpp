// TODO(Task 7): implement active expiration metadata.
#include "miniredis/storage/expiration.h"
#include "miniredis/storage/database.h"

namespace miniredis {

Expiration::Expiration(Clock& clock) : clock_(clock) {}

void Expiration::Add(const std::string&, int64_t) {
    // TODO(Task 7): push/update expiration heap entry.
}

void Expiration::Remove(const std::string&) {
    // TODO(Task 7): mark a key as no longer actively tracked.
}

size_t Expiration::ActiveExpire(Database&, size_t) {
    // TODO(Task 7): expire at most max_sample currently expired keys.
    return 0;
}

bool Expiration::HasEntry(const std::string&) const {
    // TODO(Task 7): report whether a key has live expiration metadata.
    return false;
}

}  // namespace miniredis
