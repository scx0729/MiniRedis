// TODO(Task 4): implement list command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_lpush(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_rpush(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_lpop(CommandContext&, const std::vector<std::string>&) { return RespValue::BulkString(""); }
RespValue cmd_rpop(CommandContext&, const std::vector<std::string>&) { return RespValue::BulkString(""); }
RespValue cmd_llen(CommandContext&, const std::vector<std::string>&) { return RespValue::Integer(0); }
RespValue cmd_lrange(CommandContext&, const std::vector<std::string>&) { return RespValue::MakeArray({}); }

}  // namespace miniredis
