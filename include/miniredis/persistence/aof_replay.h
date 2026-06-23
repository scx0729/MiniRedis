#pragma once

#include "miniredis/base/result.h"
#include <string>

namespace miniredis {

class Database;
class CommandRegistry;
class AofWriter;

// Replays AOF file to restore database state on startup
class AofReplay {
public:
    // Replay commands from AOF file. Returns number of commands replayed.
    // aof_writer should be null during replay to avoid re-writing replayed commands.
    static Result<int64_t> Replay(const std::string& filename, Database& db,
                                   const CommandRegistry& registry);
};

}  // namespace miniredis
