#include "test_framework.h"
#include "miniredis/storage/database.h"
#include "miniredis/command/command.h"
#include "miniredis/command/registry.h"
#include "miniredis/command/executor.h"
#include "miniredis/resp/value.h"
#include "miniredis/base/clock.h"

using namespace miniredis;

static RespValue Exec(CommandContext& ctx, const CommandRegistry& registry,
                      std::vector<std::string> args) {
    return CommandExecutor::Execute(ctx, registry, args);
}

TEST(cmd_lpush_rpush) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r1 = Exec(ctx, registry, {"LPUSH", "list", "c", "b"});
    ASSERT_EQ(r1.integer(), 2);
    auto r2 = Exec(ctx, registry, {"RPUSH", "list", "d"});
    ASSERT_EQ(r2.integer(), 3);
    auto r3 = Exec(ctx, registry, {"LPUSH", "list", "a"});
    ASSERT_EQ(r3.integer(), 4);
}

TEST(cmd_lpop_rpop) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"RPUSH", "list", "a", "b", "c"});
    auto r1 = Exec(ctx, registry, {"LPOP", "list"});
    ASSERT_EQ(r1.bulk_string(), "a");
    auto r2 = Exec(ctx, registry, {"RPOP", "list"});
    ASSERT_EQ(r2.bulk_string(), "c");
}

TEST(cmd_lpop_empty) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"LPOP", "nokey"});
    ASSERT_TRUE(r.is_null());
}

TEST(cmd_lpop_deletes_empty_key) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"RPUSH", "list", "only"});
    Exec(ctx, registry, {"LPOP", "list"});
    ASSERT_FALSE(db.Exists("list"));
}

TEST(cmd_llen) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    ASSERT_EQ(Exec(ctx, registry, {"LLEN", "nokey"}).integer(), 0);
    Exec(ctx, registry, {"RPUSH", "list", "a", "b"});
    ASSERT_EQ(Exec(ctx, registry, {"LLEN", "list"}).integer(), 2);
}

TEST(cmd_lrange) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"RPUSH", "list", "a", "b", "c", "d", "e"});
    auto r = Exec(ctx, registry, {"LRANGE", "list", "1", "3"});
    ASSERT_EQ(r.array().size(), 3u);
    ASSERT_EQ(r.array()[0]->bulk_string(), "b");
    ASSERT_EQ(r.array()[1]->bulk_string(), "c");
    ASSERT_EQ(r.array()[2]->bulk_string(), "d");
}

TEST(cmd_lrange_negative) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"RPUSH", "list", "a", "b", "c"});
    auto r = Exec(ctx, registry, {"LRANGE", "list", "-2", "-1"});
    ASSERT_EQ(r.array().size(), 2u);
    ASSERT_EQ(r.array()[0]->bulk_string(), "b");
    ASSERT_EQ(r.array()[1]->bulk_string(), "c");
}

TEST(cmd_lrange_all) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"RPUSH", "list", "a", "b", "c"});
    auto r = Exec(ctx, registry, {"LRANGE", "list", "0", "-1"});
    ASSERT_EQ(r.array().size(), 3u);
}

TEST(cmd_lrange_out_of_range) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"RPUSH", "list", "a", "b"});
    auto r = Exec(ctx, registry, {"LRANGE", "list", "0", "100"});
    ASSERT_EQ(r.array().size(), 2u);
}

TEST(cmd_lrange_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"LRANGE", "nokey", "0", "-1"});
    ASSERT_EQ(r.array().size(), 0u);
}

TEST(cmd_llen_wrong_type) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"LLEN", "key"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST_MAIN()
