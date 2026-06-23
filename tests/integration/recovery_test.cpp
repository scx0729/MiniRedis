#include "test_framework.h"
#include "miniredis/persistence/aof.h"
#include "miniredis/persistence/aof_replay.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include "miniredis/command/registry.h"
#include "miniredis/command/command.h"
#include "miniredis/command/executor.h"
#include "miniredis/base/clock.h"
#include <cstdio>

using namespace miniredis;

static const char* TEST_AOF = "/tmp/miniredis_recovery_test.aof";

static RespValue Exec(CommandContext& ctx, const CommandRegistry& registry,
                      std::vector<std::string> args) {
    return CommandExecutor::Execute(ctx, registry, args);
}

TEST(recovery_basic) {
    std::remove(TEST_AOF);

    // Phase 1: Write data and log to AOF
    {
        FakeClock clock;
        Expiration exp(clock);
        Database db(clock, &exp);
        CommandRegistry registry;
        AofWriter aof(TEST_AOF);
        CommandContext ctx{&db, &aof};

        Exec(ctx, registry, {"SET", "key1", "val1"});
        aof.Write("*3\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$4\r\nval1\r\n");

        Exec(ctx, registry, {"SET", "key2", "val2"});
        aof.Write("*3\r\n$3\r\nSET\r\n$4\r\nkey2\r\n$4\r\nval2\r\n");

        Exec(ctx, registry, {"LPUSH", "list", "a", "b", "c"});
        aof.Write("*5\r\n$5\r\nLPUSH\r\n$4\r\nlist\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n");
    }

    // Phase 2: Recover from AOF
    {
        FakeClock clock;
        Expiration exp(clock);
        Database db(clock, &exp);
        CommandRegistry registry;

        auto result = AofReplay::Replay(TEST_AOF, db, registry);
        ASSERT_TRUE(result.ok());
        ASSERT_EQ(result.value(), 3);

        ASSERT_TRUE(db.Exists("key1"));
        ASSERT_EQ(db.Get("key1").value().value(), "val1");
        ASSERT_TRUE(db.Exists("key2"));
        ASSERT_EQ(db.Get("key2").value().value(), "val2");
        ASSERT_EQ(db.LLen("list").value(), 3);
    }

    std::remove(TEST_AOF);
}

TEST(recovery_with_truncated_tail) {
    std::remove(TEST_AOF);

    {
        AofWriter aof(TEST_AOF);
        aof.Write("*3\r\n$3\r\nSET\r\n$1\r\na\r\n$1\r\n1\r\n");
        aof.Write("*3\r\n$3\r\nSET\r\n$1\r\nb\r\n$1\r\n2\r\n");
        // Truncated command
        aof.Write("*3\r\n$3\r\nSET\r\n$1\r\nc\r\n");
    }

    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;

    auto result = AofReplay::Replay(TEST_AOF, db, registry);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value(), 2);  // only first two commands replay

    ASSERT_TRUE(db.Exists("a"));
    ASSERT_TRUE(db.Exists("b"));
    ASSERT_FALSE(db.Exists("c"));

    std::remove(TEST_AOF);
}

TEST(recovery_preserves_order) {
    std::remove(TEST_AOF);

    {
        AofWriter aof(TEST_AOF);
        aof.Write("*3\r\n$3\r\nSET\r\n$7\r\ncounter\r\n$1\r\n0\r\n");
        aof.Write("*2\r\n$4\r\nINCR\r\n$7\r\ncounter\r\n");
        aof.Write("*2\r\n$4\r\nINCR\r\n$7\r\ncounter\r\n");
        aof.Write("*2\r\n$4\r\nINCR\r\n$7\r\ncounter\r\n");
    }

    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;

    auto result = AofReplay::Replay(TEST_AOF, db, registry);
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value(), 4);

    ASSERT_EQ(db.Get("counter").value().value(), "3");

    std::remove(TEST_AOF);
}

TEST(recovery_no_double_aof_write) {
    std::remove(TEST_AOF);

    // Write initial AOF
    {
        AofWriter aof(TEST_AOF);
        aof.Write("*3\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$4\r\nval1\r\n");
    }

    // Replay with aof=nullptr (should not write)
    {
        FakeClock clock;
        Expiration exp(clock);
        Database db(clock, &exp);
        CommandRegistry registry;

        AofReplay::Replay(TEST_AOF, db, registry);
    }

    // Verify file hasn't grown
    FILE* f = fopen(TEST_AOF, "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    ASSERT_EQ(size, 33L);  // "*3\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$4\r\nval1\r\n" = 33 bytes

    std::remove(TEST_AOF);
}

TEST_MAIN()
