// TODO(Task 3): implement generic and string command handlers.
#include "miniredis/command/command.h"
#include "miniredis/resp/value.h"
#include <cstddef>
#include <cstdint>
#include <optional>

namespace miniredis {

RespValue cmd_ping([[maybe_unused]] CommandContext& ctx, const std::vector<std::string>& args) {
    // Provided as a tiny working command for smoke tests while early tasks are incomplete.
    if (args.size() > 1) return RespValue::BulkString(args[1]);
    return RespValue::SimpleString("PONG");
}

RespValue cmd_echo([[maybe_unused]] CommandContext& ctx, const std::vector<std::string>& args) {
    return RespValue::BulkString(args[1]);
}

RespValue cmd_quit([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    return RespValue::SimpleString("OK");
}

RespValue cmd_del([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    std::vector<std::string> keys(args.begin() + 1, args.end());
    auto result = ctx.db->Del(keys);
    return RespValue::Integer(result);
}

RespValue cmd_exists([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    int64_t count = 0;
    std::vector<std::string> keys(args.begin() + 1, args.end());
    for(auto &key : keys){
        if(ctx.db->Exists(key)){
            count++;
        }
    }
    return RespValue::Integer(count);
}

RespValue cmd_type([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->Type(args[1]);
    if(result.ok()) { return RespValue::SimpleString(result.value()); }
    return RespValue::Error(result.error_message());
}

RespValue cmd_set([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    const std::string& key = args[1];
    const std::string& value = args[2];

    int64_t expire_ms = -1;
    bool nx = false;
    bool xx = false;

    // Parse options
    for (size_t i = 3; i < args.size(); ++i) {
        std::string opt = args[i];
        for (auto& c : opt) c = static_cast<char>(toupper(c));

        if (opt == "EX") {
            if (i + 1 >= args.size()) return RespValue::Error("ERR syntax error");
            char* end = nullptr;
            long long val = strtoll(args[i + 1].c_str(), &end, 10);
            if (*end != '\0' || val <= 0) {
                return RespValue::Error("ERR invalid expire time in 'set' command");
            }
            expire_ms = val * 1000;
            ++i;
        } else if (opt == "PX") {
            if (i + 1 >= args.size()) return RespValue::Error("ERR syntax error");
            char* end = nullptr;
            long long val = strtoll(args[i + 1].c_str(), &end, 10);
            if (*end != '\0' || val <= 0) {
                return RespValue::Error("ERR invalid expire time in 'set' command");
            }
            expire_ms = val;
            ++i;
        } else if (opt == "NX") {
            nx = true;
            xx = false;
        } else if (opt == "XX") {
            xx = true;
            nx = false;
        } else {
            return RespValue::Error("ERR syntax error");
        }
    }

    auto result = ctx.db->Set(key, value, expire_ms, nx, xx);
    if (!result.ok()) {
        return RespValue::Error(result.error_message());
    }
    return RespValue::SimpleString("OK");
}

RespValue cmd_get([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->Get(args[1]);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    if(result.value().has_value()) { return RespValue::BulkString(result.value().value()); }
    return RespValue::NullBulkString();
}

RespValue cmd_mset([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    if(args.size() % 2 == 0) { return RespValue::Error("ERR wrong number of arguments for 'mset' command"); }
    std::vector<std::pair<std::string, std::string>> kvs;
    for(size_t i = 1;i + 1 < args.size();i += 2){
        kvs.emplace_back(args[i], args[i+1]);
    }
    auto result = ctx.db->MSet(kvs);
    if(!result.ok()) { return RespValue::Error(result.error_message()); }
    return RespValue::SimpleString("OK");
}

RespValue cmd_mget([[maybe_unused]] CommandContext& ctx, const std::vector<std::string>& args) {
    std::vector<std::string> keys(args.begin() + 1, args.end());
    auto result = ctx.db->MGet(keys);
    RespValue::ArrayT arr;
    for (size_t i = 0; i < result.value().size(); ++i){
        if(result.value()[i].has_value()){
            arr.push_back(RespValue::BulkString(result.value()[i].value()));
        } else {
            arr.push_back(std::nullopt);
        }
    }
    return RespValue::MakeArray(std::move(arr));
}

RespValue cmd_append([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->Append(args[1], args[2]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    return RespValue::Integer(result.value());
}

RespValue cmd_strlen([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->Strlen(args[1]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    return RespValue::Integer(result.value());
}

RespValue cmd_incr([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->Incr(args[1]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    return RespValue::Integer(result.value());
}

RespValue cmd_decr([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto result = ctx.db->Decr(args[1]);
    if(!result.ok()) return RespValue::Error(result.error_message());
    return RespValue::Integer(result.value());
}

}  // namespace miniredis
