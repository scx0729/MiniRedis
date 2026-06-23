#include "test_framework.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
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

TEST(cmd_ping) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"PING"});
    ASSERT_EQ(r.type(), RespType::kSimpleString);
    ASSERT_EQ(r.simple_string(), "PONG");
}

TEST(cmd_ping_with_arg) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"PING", "hello"});
    ASSERT_EQ(r.type(), RespType::kBulkString);
    ASSERT_EQ(r.bulk_string(), "hello");
}

TEST(cmd_echo) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"ECHO", "world"});
    ASSERT_EQ(r.bulk_string(), "world");
}

TEST(cmd_set_get) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "value"});
    auto r = Exec(ctx, registry, {"GET", "key"});
    ASSERT_EQ(r.bulk_string(), "value");
}

TEST(cmd_get_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"GET", "nokey"});
    ASSERT_TRUE(r.is_null());
}

TEST(cmd_set_nx) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "first", "NX"});
    auto r1 = Exec(ctx, registry, {"GET", "key"});
    ASSERT_EQ(r1.bulk_string(), "first");

    Exec(ctx, registry, {"SET", "key", "second", "NX"});
    auto r2 = Exec(ctx, registry, {"GET", "key"});
    ASSERT_EQ(r2.bulk_string(), "first");  // unchanged
}

TEST(cmd_set_xx) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val", "XX"});
    auto r1 = Exec(ctx, registry, {"GET", "key"});
    ASSERT_TRUE(r1.is_null());  // key didn't exist

    Exec(ctx, registry, {"SET", "key", "first"});
    Exec(ctx, registry, {"SET", "key", "second", "XX"});
    auto r2 = Exec(ctx, registry, {"GET", "key"});
    ASSERT_EQ(r2.bulk_string(), "second");
}

TEST(cmd_del) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "a", "1"});
    Exec(ctx, registry, {"SET", "b", "2"});
    auto r = Exec(ctx, registry, {"DEL", "a", "b", "c"});
    ASSERT_EQ(r.integer(), 2);
}

TEST(cmd_exists) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "a", "1"});
    auto r = Exec(ctx, registry, {"EXISTS", "a", "b"});
    ASSERT_EQ(r.integer(), 1);
}

TEST(cmd_type) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"TYPE", "key"});
    ASSERT_EQ(r.simple_string(), "string");
}

TEST(cmd_type_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"TYPE", "nokey"});
    ASSERT_EQ(r.simple_string(), "none");
}

TEST(cmd_mset_mget) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"MSET", "a", "1", "b", "2"});
    auto r = Exec(ctx, registry, {"MGET", "a", "b", "c"});
    ASSERT_EQ(r.type(), RespType::kArray);
    ASSERT_EQ(r.array().size(), 3u);
    ASSERT_EQ(r.array()[0]->bulk_string(), "1");
    ASSERT_EQ(r.array()[1]->bulk_string(), "2");
    ASSERT_FALSE(r.array()[2].has_value());
}

TEST(cmd_append) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "hel"});
    auto r = Exec(ctx, registry, {"APPEND", "key", "lo"});
    ASSERT_EQ(r.integer(), 5);
}

TEST(cmd_strlen) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "hello"});
    auto r = Exec(ctx, registry, {"STRLEN", "key"});
    ASSERT_EQ(r.integer(), 5);
}

TEST(cmd_incr_decr) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r1 = Exec(ctx, registry, {"INCR", "counter"});
    ASSERT_EQ(r1.integer(), 1);
    auto r2 = Exec(ctx, registry, {"INCR", "counter"});
    ASSERT_EQ(r2.integer(), 2);
    auto r3 = Exec(ctx, registry, {"DECR", "counter"});
    ASSERT_EQ(r3.integer(), 1);
}

TEST(cmd_incr_non_integer) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "abc"});
    auto r = Exec(ctx, registry, {"INCR", "key"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST(cmd_wrong_type) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"LLEN", "key"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST(cmd_unknown) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"FOOBAR"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST(cmd_wrong_arity) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"GET"});
    ASSERT_EQ(r.type(), RespType::kError);
}

TEST_MAIN()
