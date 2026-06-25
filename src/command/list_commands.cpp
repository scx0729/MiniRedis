// TODO(Task 4): implement list command handlers.
#include "miniredis/command/command.h"
#include "miniredis/resp/value.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace miniredis {

RespValue cmd_lpush([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    std::vector<std::string> elements;
    for(size_t i = 2;i < args.size();i++){
        elements.push_back(args[i]);
    }
    auto result = ctx.db->LPush(args[1], elements);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    return RespValue::Integer(result.value());
}
RespValue cmd_rpush([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    std::vector<std::string> elements;
    for(size_t i = 2;i < args.size();i++){
        elements.push_back(args[i]);
    }
    auto result = ctx.db->RPush(args[1], elements);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    return RespValue::Integer(result.value());
}
RespValue cmd_lpop([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->LPop(args[1]);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    if(!result.value().has_value()) { return RespValue::NullBulkString(); }
    return RespValue::BulkString(result.value().value());
}
RespValue cmd_rpop([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->RPop(args[1]);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    if(!result.value().has_value()) { return RespValue::NullBulkString(); }
    return RespValue::BulkString(result.value().value());
}
RespValue cmd_llen(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->LLen(args[1]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}
RespValue cmd_lrange([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto start = static_cast<int64_t>(std::stoll(args[2]));
    auto stop = static_cast<int64_t>(std::stoll(args[3]));
    auto result = ctx.db->LRange(args[1], start, stop);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    RespValue::ArrayT arr;
    auto elements = result.value();
    for(auto &element : elements){
        arr.push_back(RespValue::BulkString(element));
    }
    return RespValue::MakeArray(std::move(arr));
}

}  // namespace miniredis
