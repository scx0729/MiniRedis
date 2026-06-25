// TODO(Task 4): implement hash command handlers.
#include "miniredis/command/command.h"
#include "miniredis/resp/value.h"
#include <cstddef>
#include <utility>
#include <vector>

namespace miniredis {

RespValue cmd_hset([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    if(args.size() % 2 != 0) { return RespValue::Error("args number is error"); }
    std::vector<std::pair<std::string, std::string>> vector;
    for(size_t i = 2;i < args.size() - 1;i += 2){
        vector.push_back({args[i], args[i + 1]});
    }
    auto result = ctx.db->HSet(args[1], vector);
    return RespValue::Integer(result.value());
}
RespValue cmd_hget([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->HGet(args[1], args[2]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    if(!result.value().has_value()) return RespValue::NullBulkString();
    return RespValue::BulkString(result.value().value());
}
RespValue cmd_hdel([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    std::vector<std::string> vector;
    for(size_t i = 2;i < args.size();i++){
        vector.push_back(args[i]);
    }
    auto result = ctx.db->HDel(args[1], vector);
    if(!result.ok()) return RespValue::Error(result.error_message());
    return RespValue::Integer(result.value());
}
RespValue cmd_hlen([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->HLen(args[1]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    return RespValue::Integer(result.value());
}
RespValue cmd_hgetall([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->HGetAll(args[1]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    auto &fields = result.value();
    RespValue::ArrayT array;
    for(auto &field : fields){
        array.push_back(RespValue::BulkString(field));
    }
    return RespValue::MakeArray(array);
}

}  // namespace miniredis
