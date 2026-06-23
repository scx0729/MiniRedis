#include "miniredis/command/command.h"
#include "miniredis/storage/database.h"

namespace miniredis {

RespValue cmd_lpush(CommandContext& ctx, const std::vector<std::string>& args) {
    std::vector<std::string> elements(args.begin() + 2, args.end());
    auto result = ctx.db->LPush(args[1], elements);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}

RespValue cmd_rpush(CommandContext& ctx, const std::vector<std::string>& args) {
    std::vector<std::string> elements(args.begin() + 2, args.end());
    auto result = ctx.db->RPush(args[1], elements);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}

RespValue cmd_lpop(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->LPop(args[1]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    if (!result.value().has_value()) {
        return RespValue::NullBulkString();
    }
    return RespValue::BulkString(std::move(result.value().value()));
}

RespValue cmd_rpop(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->RPop(args[1]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    if (!result.value().has_value()) {
        return RespValue::NullBulkString();
    }
    return RespValue::BulkString(std::move(result.value().value()));
}

RespValue cmd_llen(CommandContext& ctx, const std::vector<std::string>& args) {
    auto result = ctx.db->LLen(args[1]);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::Integer(result.value());
}

RespValue cmd_lrange(CommandContext& ctx, const std::vector<std::string>& args) {
    char* end1 = nullptr;
    long long start = strtoll(args[2].c_str(), &end1, 10);
    if (*end1 != '\0') {
        return RespValue::Error("ERR value is not an integer or out of range");
    }

    char* end2 = nullptr;
    long long stop = strtoll(args[3].c_str(), &end2, 10);
    if (*end2 != '\0') {
        return RespValue::Error("ERR value is not an integer or out of range");
    }

    auto result = ctx.db->LRange(args[1], start, stop);
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
