#pragma once

#include "command.h"
#include "registry.h"
#include "miniredis/resp/value.h"
#include <string>
#include <vector>

namespace miniredis {

class CommandExecutor {
public:
    // Execute a parsed command (args[0] = command name)
    static RespValue Execute(CommandContext& ctx, const CommandRegistry& registry,
                             const std::vector<std::string>& args);
};

}  // namespace miniredis
