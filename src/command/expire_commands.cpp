// TODO(Task 7): implement TTL command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_expire(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_pexpire(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_ttl(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(-2); }
RespValue cmd_pttl(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(-2); }
RespValue cmd_persist(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }

}  // namespace miniredis
