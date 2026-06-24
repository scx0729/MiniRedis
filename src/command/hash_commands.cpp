// TODO(Task 4): implement hash command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_hset([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_hget([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::BulkString(""); }
RespValue cmd_hdel([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_hlen([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::Integer(0); }
RespValue cmd_hgetall([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) { return RespValue::MakeArray({}); }

}  // namespace miniredis
