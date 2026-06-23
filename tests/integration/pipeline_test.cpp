#include "test_framework.h"
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

static std::string MakeCommand(std::vector<std::string> args) {
    RespValue::ArrayT arr;
    for (const auto& a : args) arr.push_back(RespValue::BulkString(a));
    return RespEncoder::Encode(RespValue::MakeArray(std::move(arr)));
}

TEST(pipeline_mixed_commands) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    std::string pipeline;
    pipeline += MakeCommand({"SET", "x", "10"});
    pipeline += MakeCommand({"INCR", "x"});
    pipeline += MakeCommand({"INCR", "x"});
    pipeline += MakeCommand({"GET", "x"});

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
    ASSERT_EQ(responses[1].integer(), 11);
    ASSERT_EQ(responses[2].integer(), 12);
    ASSERT_EQ(responses[3].bulk_string(), "12");
}

TEST(pipeline_incremental_parse) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    std::string cmd1 = MakeCommand({"SET", "a", "1"});
    std::string cmd2 = MakeCommand({"GET", "a"});

    RespParser parser;
    std::vector<RespValue> responses;

    auto r1 = parser.Parse(cmd1);
    ASSERT_EQ(r1.status, ParseStatus::kComplete);
    {
        const auto& arr = r1.value->array();
        std::vector<std::string> args;
        for (const auto& elem : arr) args.push_back(elem->bulk_string());
        responses.push_back(CommandExecutor::Execute(ctx, registry, args));
    }

    auto r2 = parser.Parse(cmd2);
    ASSERT_EQ(r2.status, ParseStatus::kComplete);
    {
        const auto& arr = r2.value->array();
        std::vector<std::string> args;
        for (const auto& elem : arr) args.push_back(elem->bulk_string());
        responses.push_back(CommandExecutor::Execute(ctx, registry, args));
    }

    ASSERT_EQ(responses.size(), 2u);
    ASSERT_EQ(responses[0].simple_string(), "OK");
    ASSERT_EQ(responses[1].bulk_string(), "1");
}

TEST_MAIN()
