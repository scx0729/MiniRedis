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

TEST(cmd_hset_hget) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r1 = Exec(ctx, registry, {"HSET", "user", "name", "alice"});
    ASSERT_EQ(r1.integer(), 1);
    auto r2 = Exec(ctx, registry, {"HSET", "user", "age", "20"});
    ASSERT_EQ(r2.integer(), 1);

    auto g = Exec(ctx, registry, {"HGET", "user", "name"});
    ASSERT_EQ(g.bulk_string(), "alice");
}

TEST(cmd_hset_update) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"HSET", "user", "name", "alice"});
    auto r = Exec(ctx, registry, {"HSET", "user", "name", "bob"});
    ASSERT_EQ(r.integer(), 0);  // updated, not new
}

TEST(cmd_hget_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"HGET", "nokey", "field"});
    ASSERT_TRUE(r.is_null());
}

TEST(cmd_hdel) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"HSET", "user", "a", "1", "b", "2", "c", "3"});
    auto r = Exec(ctx, registry, {"HDEL", "user", "a", "x"});
    ASSERT_EQ(r.integer(), 1);
}

TEST(cmd_hlen) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    ASSERT_EQ(Exec(ctx, registry, {"HLEN", "nokey"}).integer(), 0);
    Exec(ctx, registry, {"HSET", "user", "a", "1", "b", "2"});
    ASSERT_EQ(Exec(ctx, registry, {"HLEN", "user"}).integer(), 2);
}

TEST(cmd_hgetall) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"HSET", "user", "name", "alice", "age", "20"});
    auto r = Exec(ctx, registry, {"HGETALL", "user"});
    ASSERT_EQ(r.array().size(), 4u);
    // Sorted order: age, name
    ASSERT_EQ(r.array()[0]->bulk_string(), "age");
    ASSERT_EQ(r.array()[1]->bulk_string(), "20");
    ASSERT_EQ(r.array()[2]->bulk_string(), "name");
    ASSERT_EQ(r.array()[3]->bulk_string(), "alice");
}

TEST(cmd_hgetall_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"HGETALL", "nokey"});
    ASSERT_EQ(r.array().size(), 0u);
}

TEST(cmd_hlen_wrong_type) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"HLEN", "key"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST(cmd_hset_wrong_arity) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"HSET", "user", "field"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST(cmd_hdel_removes_empty_key) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"HSET", "user", "only", "val"});
    Exec(ctx, registry, {"HDEL", "user", "only"});
    ASSERT_FALSE(db.Exists("user"));
}

TEST_MAIN()
