// TODO(Task 5, Task 8): implement session command processing and AOF logging.
#include "miniredis/server/session.h"

namespace miniredis {

Session::Session(Connection& conn, Database& db, const CommandRegistry& registry,
                 CommandContext& cmd_ctx, ParserLimits parser_limits)
    : conn_(conn), db_(db), registry_(registry), cmd_ctx_(cmd_ctx),
      parser_(parser_limits) {}

bool Session::ProcessInput() {
    // TODO(Task 5): parse all complete commands from conn_.InputBuffer(), execute them,
    // encode responses into conn_.OutputBuffer(), and handle protocol errors/QUIT.
    // TODO(Task 8): after successful write commands, append RESP commands to AOF.
    return false;
}

}  // namespace miniredis
