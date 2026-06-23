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

TEST(cmd_expire) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"EXPIRE", "key", "10"});
    ASSERT_EQ(r.integer(), 1);

    auto t = Exec(ctx, registry, {"TTL", "key"});
    ASSERT_EQ(t.integer(), 10);
}

TEST(cmd_expire_nonexistent) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"EXPIRE", "nokey", "10"});
    ASSERT_EQ(r.integer(), 0);
}

TEST(cmd_pexpire) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"PEXPIRE", "key", "5000"});
    ASSERT_EQ(r.integer(), 1);

    auto t = Exec(ctx, registry, {"PTTL", "key"});
    ASSERT_EQ(t.integer(), 5000);
}

TEST(cmd_ttl_no_expiry) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"TTL", "key"});
    ASSERT_EQ(r.integer(), -1);
}

TEST(cmd_ttl_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"TTL", "nokey"});
    ASSERT_EQ(r.integer(), -2);
}

TEST(cmd_persist) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    Exec(ctx, registry, {"EXPIRE", "key", "10"});
    auto r = Exec(ctx, registry, {"PERSIST", "key"});
    ASSERT_EQ(r.integer(), 1);

    auto t = Exec(ctx, registry, {"TTL", "key"});
    ASSERT_EQ(t.integer(), -1);
}

TEST(cmd_persist_no_expiry) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    auto r = Exec(ctx, registry, {"PERSIST", "key"});
    ASSERT_EQ(r.integer(), 0);
}

TEST(cmd_persist_nonexistent) {
    FakeClock clock;
    Database db(clock);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    auto r = Exec(ctx, registry, {"PERSIST", "nokey"});
    ASSERT_EQ(r.integer(), 0);
}

TEST(cmd_ttl_expired_key) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val"});
    Exec(ctx, registry, {"EXPIRE", "key", "5"});

    clock.AdvanceMs(6000);

    auto r = Exec(ctx, registry, {"GET", "key"});
    ASSERT_TRUE(r.is_null());

    auto t = Exec(ctx, registry, {"TTL", "key"});
    ASSERT_EQ(t.integer(), -2);
}

TEST(cmd_set_ex_option) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val", "EX", "30"});
    auto t = Exec(ctx, registry, {"TTL", "key"});
    ASSERT_EQ(t.integer(), 30);
}

TEST(cmd_set_px_option) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val", "PX", "5000"});
    auto t = Exec(ctx, registry, {"PTTL", "key"});
    ASSERT_EQ(t.integer(), 5000);
}

TEST(cmd_set_clears_ttl) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);
    CommandRegistry registry;
    CommandContext ctx{&db, nullptr};

    Exec(ctx, registry, {"SET", "key", "val", "EX", "30"});
    Exec(ctx, registry, {"SET", "key", "newval"});
    auto t = Exec(ctx, registry, {"TTL", "key"});
    ASSERT_EQ(t.integer(), -1);
}

TEST_MAIN()
