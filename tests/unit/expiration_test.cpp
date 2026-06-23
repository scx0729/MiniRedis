#include "test_framework.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include "miniredis/base/clock.h"

using namespace miniredis;

TEST(expire_basic) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("key", "value");
    db.SetExpireMs("key", 1000);

    // Not expired yet
    ASSERT_TRUE(db.Exists("key"));
    ASSERT_EQ(db.GetExpireMs("key"), 1000);

    // Advance time past expiry
    clock.AdvanceMs(1001);
    ASSERT_FALSE(db.Exists("key"));
}

TEST(ttl_nonexistent) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.GetExpireMs("nokey"), -2);
}

TEST(ttl_no_expiry) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "val");
    ASSERT_EQ(db.GetExpireMs("key"), -1);
}

TEST(ttl_remaining) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("key", "val");
    db.SetExpireMs("key", 5000);

    clock.AdvanceMs(2000);
    ASSERT_EQ(db.GetExpireMs("key"), 3000);
}

TEST(clear_expire) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("key", "val");
    db.SetExpireMs("key", 1000);
    db.ClearExpire("key");

    clock.AdvanceMs(2001);
    ASSERT_TRUE(db.Exists("key"));
    ASSERT_EQ(db.GetExpireMs("key"), -1);
}

TEST(set_ex_option) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("key", "val", 2000);
    ASSERT_EQ(db.GetExpireMs("key"), 2000);
}

TEST(set_clears_ttl) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("key", "val", 5000);
    // Overwrite with no TTL
    db.Set("key", "newval");
    ASSERT_EQ(db.GetExpireMs("key"), -1);
}

TEST(active_expire) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("a", "1");
    db.SetExpireMs("a", 100);
    db.Set("b", "2");
    db.SetExpireMs("b", 200);
    db.Set("c", "3");  // no expiry

    clock.AdvanceMs(150);
    size_t expired = exp.ActiveExpire(db, 100);
    ASSERT_GE(expired, 1u);
    ASSERT_FALSE(db.Exists("a"));
    ASSERT_TRUE(db.Exists("b"));  // not yet expired
    ASSERT_TRUE(db.Exists("c"));
}

TEST(expire_nonexistent_key) {
    FakeClock clock;
    Database db(clock);
    // Should not crash
    db.SetExpireMs("nokey", 1000);
}

TEST(preserve_ttl_on_incr) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    db.Set("counter", "0", 5000);
    db.Incr("counter");
    // counter should still have TTL (GetExpireMs returns remaining time)
    int64_t ttl = db.GetExpireMs("counter");
    ASSERT_GT(ttl, 0);
    ASSERT_LE(ttl, 5000);
}

TEST(expire_sample_count_limit) {
    FakeClock clock;
    Expiration exp(clock);
    Database db(clock, &exp);

    for (int i = 0; i < 10; ++i) {
        db.Set("key" + std::to_string(i), "val");
        db.SetExpireMs("key" + std::to_string(i), 100);
    }

    clock.AdvanceMs(150);
    size_t expired = exp.ActiveExpire(db, 5);
    ASSERT_EQ(expired, 5u);
}

TEST_MAIN()
