// TODO(Task 7): implement TTL command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_expire([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_pexpire([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_ttl([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(-2); }
RespValue cmd_pttl([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(-2); }
RespValue cmd_persist([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }

}  // namespace miniredis
