#include "test_framework.h"
#include "miniredis/server/session.h"
#include "miniredis/server/connection.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include "miniredis/command/registry.h"
#include "miniredis/command/command.h"
#include "miniredis/command/executor.h"
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/parser.h"
#include "miniredis/resp/value.h"
#include "miniredis/base/clock.h"

using namespace miniredis;

// Test command execution through the full pipeline (parser -> executor)
// without real sockets

static std::string MakeCommand(std::vector<std::string> args) {
    RespValue::ArrayT arr;
    for (const auto& a : args) arr.push_back(RespValue::BulkString(a));
    return RespEncoder::Encode(RespValue::MakeArray(std::move(arr)));
}

TEST(session_single_command) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    RespParser parser;
    std::string input = MakeCommand({"PING"});
    auto parse_result = parser.Parse(input);
    ASSERT_EQ(parse_result.status, ParseStatus::kComplete);

    const auto& arr = parse_result.value->array();
    std::vector<std::string> args;
    for (const auto& elem : arr) {
        args.push_back(elem->bulk_string());
    }

    auto response = CommandExecutor::Execute(ctx, registry, args);
    ASSERT_EQ(response.type(), RespType::kSimpleString);
    ASSERT_EQ(response.simple_string(), "PONG");
}

TEST(session_set_get) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    CommandExecutor::Execute(ctx, registry, {"SET", "key", "value"});
    auto r = CommandExecutor::Execute(ctx, registry, {"GET", "key"});
    ASSERT_EQ(r.bulk_string(), "value");
}

TEST(session_pipeline_simulation) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    std::string pipeline;
    pipeline += MakeCommand({"SET", "a", "1"});
    pipeline += MakeCommand({"SET", "b", "2"});
    pipeline += MakeCommand({"GET", "a"});
    pipeline += MakeCommand({"GET", "b"});

    RespParser parser;
    std::vector<RespValue> responses;

    size_t pos = 0;
    while (pos < pipeline.size()) {
        std::string_view remaining(pipeline.data() + pos, pipeline.size() - pos);
        auto result = parser.Parse(remaining);
        if (result.status != ParseStatus::kComplete) break;

        const auto& arr = result.value->array();
        std::vector<std::string> args;
        for (const auto& elem : arr) {
            args.push_back(elem->bulk_string());
        }

        responses.push_back(CommandExecutor::Execute(ctx, registry, args));
        pos += result.consumed;
    }

    ASSERT_EQ(responses.size(), 4u);
    ASSERT_EQ(responses[0].simple_string(), "OK");
    ASSERT_EQ(responses[1].simple_string(), "OK");
    ASSERT_EQ(responses[2].bulk_string(), "1");
    ASSERT_EQ(responses[3].bulk_string(), "2");
}

TEST(session_error_response) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    CommandExecutor::Execute(ctx, registry, {"SET", "key", "val"});
    auto r = CommandExecutor::Execute(ctx, registry, {"LLEN", "key"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST(session_unknown_command) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = CommandExecutor::Execute(ctx, registry, {"FOOBAR"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST_MAIN()
