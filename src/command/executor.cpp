#include "miniredis/command/executor.h"

namespace miniredis {

RespValue CommandExecutor::Execute(CommandContext& ctx, const CommandRegistry& registry,
                                   const std::vector<std::string>& args) {
    if (args.empty()) {
        return RespValue::Error("ERR empty command");
    }

    const Command* cmd = registry.Find(args[0]);
    if (!cmd) {
        return RespValue::Error("ERR unknown command '" + args[0] + "'");
    }

    // Check arity
    int arity = cmd->def.arity;
    if (arity < 0) {
        // At least abs(arity) arguments
        if (static_cast<int>(args.size()) < -arity) {
            return RespValue::Error("ERR wrong number of arguments for '" +
                                    cmd->def.name + "' command");
        }
    } else {
        // Exactly arity arguments
        if (static_cast<int>(args.size()) != arity) {
            return RespValue::Error("ERR wrong number of arguments for '" +
                                    cmd->def.name + "' command");
        }
    }

    return cmd->handler(ctx, args);
}

}  // namespace miniredis
