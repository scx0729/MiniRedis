#include "miniredis/command/command.h"
#include "miniredis/storage/database.h"
#include <cstdlib>

namespace miniredis {

RespValue cmd_expire(CommandContext& ctx, const std::vector<std::string>& args) {
    char* end = nullptr;
    long long seconds = strtoll(args[2].c_str(), &end, 10);
    if (*end != '\0' || seconds <= 0) {
        return RespValue::Error("ERR invalid expire time in 'expire' command");
    }

    if (!ctx.db->Exists(args[1])) {
        return RespValue::Integer(0);
    }

    ctx.db->SetExpireMs(args[1], seconds * 1000);
    return RespValue::Integer(1);
}

RespValue cmd_pexpire(CommandContext& ctx, const std::vector<std::string>& args) {
    char* end = nullptr;
    long long ms = strtoll(args[2].c_str(), &end, 10);
    if (*end != '\0' || ms <= 0) {
        return RespValue::Error("ERR invalid expire time in 'pexpire' command");
    }

    if (!ctx.db->Exists(args[1])) {
        return RespValue::Integer(0);
    }

    ctx.db->SetExpireMs(args[1], ms);
    return RespValue::Integer(1);
}

RespValue cmd_ttl(CommandContext& ctx, const std::vector<std::string>& args) {
    if (!ctx.db->Exists(args[1])) {
        return RespValue::Integer(-2);
    }

    int64_t remaining_ms = ctx.db->GetExpireMs(args[1]);
    if (remaining_ms == -1) {
        return RespValue::Integer(-1);  // no expiry
    }
    if (remaining_ms == -2) {
        return RespValue::Integer(-2);  // expired
    }

    // Convert to seconds (floor)
    return RespValue::Integer(remaining_ms / 1000);
}

RespValue cmd_pttl(CommandContext& ctx, const std::vector<std::string>& args) {
    if (!ctx.db->Exists(args[1])) {
        return RespValue::Integer(-2);
    }

    int64_t remaining_ms = ctx.db->GetExpireMs(args[1]);
    if (remaining_ms == -1) {
        return RespValue::Integer(-1);
    }
    if (remaining_ms == -2) {
        return RespValue::Integer(-2);
    }

    return RespValue::Integer(remaining_ms);
}

RespValue cmd_persist(CommandContext& ctx, const std::vector<std::string>& args) {
    if (!ctx.db->Exists(args[1])) {
        return RespValue::Integer(0);
    }

    int64_t remaining = ctx.db->GetExpireMs(args[1]);
    if (remaining == -1) {
        return RespValue::Integer(0);  // no expiry
    }

    ctx.db->ClearExpire(args[1]);
    return RespValue::Integer(1);
}

}  // namespace miniredis
