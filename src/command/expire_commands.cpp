// TODO(Task 7): implement TTL command handlers.
#include "miniredis/command/command.h"
#include "miniredis/resp/value.h"
#include <cstdlib>

namespace miniredis {

RespValue cmd_expire([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    char *end = nullptr;
    auto seconds = strtoll(args[2].c_str(), &end, 10);
    if(*end != '\0'){
        return RespValue::Error("ERR invalid expire time in 'expire' command");
    }
    if(seconds <= 0) {
        ctx.db->Del(args[1]);
        return RespValue::Integer(1);
    }
    if(!ctx.db->Exists(args[1])){
        return RespValue::Integer(0);
    }
    ctx.db->SetExpireMs(args[1], seconds * 1000);
    return RespValue::Integer(1);
}
RespValue cmd_pexpire([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    char *end = nullptr;
    auto mseconds = strtoll(args[2].c_str(), &end, 10);
    if(*end != '\0'){
        return RespValue::Error("ERR invalid expire time in 'pexpire' command");
    }
    if(mseconds <= 0) {
        ctx.db->Del(args[1]);
        return RespValue::Integer(1);
    }
    if(!ctx.db->Exists(args[1])){
        return RespValue::Integer(0);
    }
    ctx.db->SetExpireMs(args[1], mseconds);
    return RespValue::Integer(1);
}
RespValue cmd_ttl([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto remaining = ctx.db->GetExpireMs(args[1]);
    if(remaining == -2){
        return RespValue::Integer(-2);
    }
    if(remaining == -1){
        return RespValue::Integer(-1);
    }
    return RespValue::Integer(remaining / 1000);
}
RespValue cmd_pttl([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    auto remaining = ctx.db->GetExpireMs(args[1]);
    if(remaining == -2){
        return RespValue::Integer(-2);
    }
    if(remaining == -1){
        return RespValue::Integer(-1);
    }
    return RespValue::Integer(remaining);
}
RespValue cmd_persist([[maybe_unused]] CommandContext& ctx, [[maybe_unused]] const std::vector<std::string>& args) {
    if(ctx.db->GetExpireMs(args[1]) == -2 || ctx.db->GetExpireMs(args[1]) == -1){
        return RespValue::Integer(0);
    }
    ctx.db->ClearExpire(args[1]);
    return RespValue::Integer(1);
}

}  // namespace miniredis
