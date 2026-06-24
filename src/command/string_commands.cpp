// TODO(Task 3): implement generic and string command handlers.
#include "miniredis/command/command.h"

namespace miniredis {

RespValue cmd_ping(CommandContext&, const std::vector<std::string>& args) {
    // Provided as a tiny working command for smoke tests while early tasks are incomplete.
    if (args.size() > 1) return RespValue::BulkString(args[1]);
    return RespValue::SimpleString("PONG");
}

RespValue cmd_echo(CommandContext&, const std::vector<std::string>& args) {
    return RespValue::BulkString(args[1]);
}

RespValue cmd_quit(CommandContext&, const std::vector<std::string>&) {
    return RespValue::SimpleString("OK");
}

RespValue cmd_del(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): delete keys and return deletion count.
    return RespValue::Integer(0);
}

RespValue cmd_exists(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): count existing keys.
    return RespValue::Integer(0);
}

RespValue cmd_type(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): return none/string/list/hash.
    return RespValue::SimpleString("none");
}

RespValue cmd_set(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): implement SET options and storage update.
    return RespValue::SimpleString("OK");
}

RespValue cmd_get(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): return stored string or nil.
    return RespValue::BulkString("");
}

RespValue cmd_mset(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): set key/value pairs.
    return RespValue::SimpleString("OK");
}

RespValue cmd_mget(CommandContext&, const std::vector<std::string>& args) {
    // TODO(Task 3): return one array element per key.
    RespValue::ArrayT arr;
    for (size_t i = 1; i < args.size(); ++i) arr.push_back(RespValue::BulkString(""));
    return RespValue::MakeArray(std::move(arr));
}

RespValue cmd_append(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): append bytes and return new length.
    return RespValue::Integer(0);
}

RespValue cmd_strlen(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): return string byte length.
    return RespValue::Integer(0);
}

RespValue cmd_incr(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): parse, increment, detect overflow.
    return RespValue::Integer(0);
}

RespValue cmd_decr(CommandContext&, const std::vector<std::string>&) {
    // TODO(Task 3): parse, decrement, detect overflow.
    return RespValue::Integer(0);
}

}  // namespace miniredis
