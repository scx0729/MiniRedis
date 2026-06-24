// TODO(Task 4): implement hash command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_hset(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_hget(CommandContext&, const std::vector<std::string>&) { return RespValue::BulkString(""); }
RespValue cmd_hdel(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_hlen(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_hgetall(CommandContext&, const std::vector<std::string>&) { return RespValue::MakeArray({}); }

}  // namespace miniredis
