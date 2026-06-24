// TODO(Task 8): implement AOF replay.
#include "miniredis/persistence/aof_replay.h"

namespace miniredis {

Result<int64_t> AofReplay::Replay([[maybe_unused]] const std::string& filename, [[maybe_unused]] Database& db, [[maybe_unused]] const CommandRegistry& registry) {
    // TODO(Task 8): read RESP commands, execute them with a null AOF writer,
    // and stop cleanly at a truncated/corrupt tail.
    return Result<int64_t>::Ok(0);
}

}  // namespace miniredis
