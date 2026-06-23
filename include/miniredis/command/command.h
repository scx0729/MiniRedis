#pragma once

#include "miniredis/resp/value.h"
#include <cstdint>
#include <string>
#include <vector>

namespace miniredis {

class Database;
class AofWriter;

// Command execution context
struct CommandContext {
    Database* db = nullptr;     // not owned
    AofWriter* aof = nullptr;   // not owned, may be null if AOF disabled
};

// Command definition
struct CommandDef {
    std::string name;           // uppercase command name
    int arity;                  // number of arguments including command name. Negative = at least abs(arity)
    bool write;                 // whether this command modifies data
    bool aof;                   // whether to log to AOF on success
};

using CommandHandler = RespValue (*)(CommandContext& ctx, const std::vector<std::string>& args);

struct Command {
    CommandDef def;
    CommandHandler handler;
};

}  // namespace miniredis
