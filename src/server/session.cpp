#include "miniredis/server/session.h"
#include "miniredis/command/executor.h"
#include "miniredis/resp/encoder.h"

namespace miniredis {

Session::Session(Connection& conn, Database& db, const CommandRegistry& registry,
                 CommandContext& cmd_ctx)
    : conn_(conn), db_(db), registry_(registry), cmd_ctx_(cmd_ctx) {}

bool Session::ProcessInput() {
    while (conn_.InputBuffer().Available() > 0) {
        auto input = conn_.InputBuffer().Peek();
        auto result = parser_.Parse(input);

        if (result.status == ParseStatus::kNeedMoreData) {
            break;  // need more data
        }

        if (result.status == ParseStatus::kProtocolError) {
            // Send error and close connection
            std::string err = RespEncoder::Encode(
                RespValue::Error(result.error.message));
            conn_.QueueOutput(err);
            return false;
        }

        // Complete message parsed
        // Consume the parsed bytes
        conn_.InputBuffer().Consume(result.consumed);

        // Extract command from RESP Array
        const auto& value = *result.value;
        if (value.type() != RespType::kArray || value.is_null()) {
            std::string err = RespEncoder::Encode(
                RespValue::Error("ERR protocol error: expected array"));
            conn_.QueueOutput(err);
            return false;
        }

        const auto& arr = value.array();
        if (arr.empty()) {
            std::string err = RespEncoder::Encode(
                RespValue::Error("ERR protocol error: empty command"));
            conn_.QueueOutput(err);
            return false;
        }

        // Convert array elements to command args
        std::vector<std::string> args;
        bool valid = true;
        for (const auto& elem : arr) {
            if (!elem.has_value() || elem->type() != RespType::kBulkString) {
                valid = false;
                break;
            }
            if (elem->is_null()) {
                valid = false;
                break;
            }
            args.push_back(elem->bulk_string());
        }

        if (!valid || args.empty()) {
            std::string err = RespEncoder::Encode(
                RespValue::Error("ERR protocol error: invalid command format"));
            conn_.QueueOutput(err);
            return false;
        }

        // Execute command
        RespValue response = CommandExecutor::Execute(cmd_ctx_, registry_, args);

        // Encode response
        std::string encoded = RespEncoder::Encode(response);
        conn_.QueueOutput(encoded);

        // Handle QUIT command
        if (args[0] == "QUIT" || args[0] == "quit") {
            conn_.MarkClosing();
            return true;
        }
    }

    return true;
}

}  // namespace miniredis
