#include "miniredis/command/registry.h"
#include <algorithm>

namespace miniredis {

// Forward declarations for command handlers (defined in separate files)
// string_commands.cpp
RespValue cmd_ping(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_echo(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_quit(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_del(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_exists(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_type(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_set(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_get(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_mset(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_mget(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_append(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_strlen(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_incr(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_decr(CommandContext& ctx, const std::vector<std::string>& args);
// list_commands.cpp
RespValue cmd_lpush(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_rpush(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_lpop(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_rpop(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_llen(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_lrange(CommandContext& ctx, const std::vector<std::string>& args);
// hash_commands.cpp
RespValue cmd_hset(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_hget(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_hdel(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_hlen(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_hgetall(CommandContext& ctx, const std::vector<std::string>& args);
// expire_commands.cpp
RespValue cmd_expire(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_pexpire(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_ttl(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_pttl(CommandContext& ctx, const std::vector<std::string>& args);
RespValue cmd_persist(CommandContext& ctx, const std::vector<std::string>& args);

std::string CommandRegistry::ToUpper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

CommandRegistry::CommandRegistry() {
    // Connection commands
    Register({{"PING", -1, false, false}, cmd_ping});
    Register({{"ECHO", 2, false, false}, cmd_echo});
    Register({{"QUIT", 1, false, false}, cmd_quit});

    // Generic key commands
    Register({{"DEL", -2, true, true}, cmd_del});
    Register({{"EXISTS", -2, false, false}, cmd_exists});
    Register({{"TYPE", 2, false, false}, cmd_type});

    // String commands
    Register({{"SET", -3, true, true}, cmd_set});
    Register({{"GET", 2, false, false}, cmd_get});
    Register({{"MSET", -3, true, true}, cmd_mset});
    Register({{"MGET", -2, false, false}, cmd_mget});
    Register({{"APPEND", 3, true, true}, cmd_append});
    Register({{"STRLEN", 2, false, false}, cmd_strlen});
    Register({{"INCR", 2, true, true}, cmd_incr});
    Register({{"DECR", 2, true, true}, cmd_decr});

    // List commands
    Register({{"LPUSH", -3, true, true}, cmd_lpush});
    Register({{"RPUSH", -3, true, true}, cmd_rpush});
    Register({{"LPOP", 2, true, true}, cmd_lpop});
    Register({{"RPOP", 2, true, true}, cmd_rpop});
    Register({{"LLEN", 2, false, false}, cmd_llen});
    Register({{"LRANGE", 4, false, false}, cmd_lrange});

    // Hash commands
    Register({{"HSET", -4, true, true}, cmd_hset});
    Register({{"HGET", 3, false, false}, cmd_hget});
    Register({{"HDEL", -3, true, true}, cmd_hdel});
    Register({{"HLEN", 2, false, false}, cmd_hlen});
    Register({{"HGETALL", 2, false, false}, cmd_hgetall});

    // TTL commands
    Register({{"EXPIRE", 3, true, true}, cmd_expire});
    Register({{"PEXPIRE", 3, true, true}, cmd_pexpire});
    Register({{"TTL", 2, false, false}, cmd_ttl});
    Register({{"PTTL", 2, false, false}, cmd_pttl});
    Register({{"PERSIST", 2, true, true}, cmd_persist});
}

const Command* CommandRegistry::Find(const std::string& name) const {
    auto it = commands_.find(ToUpper(name));
    if (it == commands_.end()) return nullptr;
    return &it->second;
}

void CommandRegistry::Register(Command cmd) {
    std::string upper = ToUpper(cmd.def.name);
    cmd.def.name = upper;
    commands_.emplace(upper, std::move(cmd));
}

}  // namespace miniredis
