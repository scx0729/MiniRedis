#pragma once

#include "connection.h"
#include "miniredis/command/command.h"
#include "miniredis/command/registry.h"
#include "miniredis/resp/parser.h"
#include "miniredis/storage/database.h"

namespace miniredis {

// Session manages the command processing loop for a single connection.
// It is decoupled from socket I/O - it reads from input buffer and writes to output buffer.
class Session {
public:
    Session(Connection& conn, Database& db, const CommandRegistry& registry,
            CommandContext& cmd_ctx, ParserLimits parser_limits = {});

    // Process available input: parse commands, execute, encode responses.
    // Returns false if the connection should be closed.
    bool ProcessInput();

private:
    Connection& conn_;
    Database& db_;
    const CommandRegistry& registry_;
    CommandContext& cmd_ctx_;
    RespParser parser_;
};

}  // namespace miniredis
