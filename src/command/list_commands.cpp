// TODO(Task 4): implement list command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_lpush([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_rpush([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_lpop([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::BulkString(""); }
RespValue cmd_rpop([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::BulkString(""); }
RespValue cmd_llen([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_lrange([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::MakeArray({}); }

}  // namespace miniredis
