#include "miniredis/server/session.h"
#include "miniredis/command/executor.h"
#include "miniredis/persistence/aof.h"
#include "miniredis/resp/encoder.h"

namespace miniredis {

namespace {

std::string EncodeCommandForAof(const std::vector<std::string>& args) {
    RespValue::ArrayT values;
    values.reserve(args.size());
    for (const auto& arg : args) {
        values.push_back(RespValue::BulkString(arg));
    }
    return RespEncoder::Encode(RespValue::MakeArray(std::move(values)));
}

}  // namespace

Session::Session(Connection& conn, Database& db, const CommandRegistry& registry,
                 CommandContext& cmd_ctx, ParserLimits parser_limits)
    : conn_(conn), db_(db), registry_(registry), cmd_ctx_(cmd_ctx),
      parser_(parser_limits) {}

bool Session::ProcessInput() {
    while (conn_.InputBuffer().Available() > 0 || parser_.HasBufferedData()) {
        auto input = conn_.InputBuffer().Peek();
        auto result = parser_.Parse(input);
        if (!input.empty()) {
            auto consume_result = conn_.InputBuffer().Consume(input.size());
            if (!consume_result.ok()) {
                return false;
            }
        }

        if (result.status == ParseStatus::kNeedMoreData) {
            break;
        }

        if (result.status == ParseStatus::kProtocolError) {
            std::string err = RespEncoder::Encode(RespValue::Error(result.error.message));
            conn_.QueueOutput(err);
            return false;
        }

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

        std::vector<std::string> args;
        bool valid = true;
        for (const auto& elem : arr) {
            if (!elem.has_value() || elem->type() != RespType::kBulkString || elem->is_null()) {
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

        RespValue response = CommandExecutor::Execute(cmd_ctx_, registry_, args);

        const Command* command = registry_.Find(args[0]);
        if (command != nullptr && command->def.aof && cmd_ctx_.aof != nullptr &&
            response.type() != RespType::kError) {
            auto write_result = cmd_ctx_.aof->Write(EncodeCommandForAof(args));
            if (!write_result.ok()) {
                response = RespValue::Error("ERR AOF write failed: " +
                                            write_result.error_message());
            }
        }

        std::string encoded = RespEncoder::Encode(response);
        if (!conn_.QueueOutput(encoded)) {
            return false;
        }

        if (args[0] == "QUIT" || args[0] == "quit") {
            conn_.MarkClosing();
            return true;
        }
    }

    return true;
}

}  // namespace miniredis
