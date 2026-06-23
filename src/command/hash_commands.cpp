#include "miniredis/command/command.h"
#include "miniredis/storage/database.h"

namespace miniredis {

RespValue cmd_hset(CommandContext& ctx, const std::vector<std::string>& args) {
    if ((args.size() - 2) % 2 != 0) {
        return RespValue::Error("ERR wrong number of arguments for 'hset' command");
    }

    std::vector<std::pair<std::string, std::string>> fvs;
    for (size_t i = 2; i < args.size(); i += 2) {
        fvs.emplace_back(args[i], args[i + 1]);
    }

    auto result = ctx.db->HSet(args[1], fvs);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}

RespValue cmd_hget(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->HGet(args[1], args[2]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    if (!result.value().has_value()) {
        return RespValue::NullBulkString();
    }
    return RespValue::BulkString(std::move(result.value().value()));
}

RespValue cmd_hdel(CommandContext& ctx, const std::vector<std::string>& args) {
    std::vector<std::string> fields(args.begin() + 2, args.end());
    auto result = ctx.db->HDel(args[1], fields);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}

RespValue cmd_hlen(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->HLen(args[1]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}

RespValue cmd_hgetall(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->HGetAll(args[1]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }

    RespValue::ArrayT arr;
    for (auto& elem : result.value()) {
        arr.push_back(RespValue::BulkString(std::move(elem)));
    }
    return RespValue::MakeArray(std::move(arr));
}

}  // namespace miniredis
