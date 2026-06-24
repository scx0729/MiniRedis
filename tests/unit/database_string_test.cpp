#include "test_framework.h"
#include "miniredis/storage/database.h"
#include "miniredis/base/clock.h"

using namespace miniredis;

TEST(db_set_get_string) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "value");
    auto r = db.Get("key");
    ASSERT_TRUE(r.ok());
    ASSERT_TRUE(r.value().has_value());
    ASSERT_EQ(r.value().value(), "value");
}

TEST(db_get_nonexistent) {
    FakeClock clock;
    Database db(clock);
    auto r = db.Get("nokey");
    ASSERT_TRUE(r.ok());
    ASSERT_FALSE(r.value().has_value());
}

TEST(db_del_existing) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "val");
    ASSERT_EQ(db.Del("key"), 1);
    ASSERT_FALSE(db.Exists("key"));
}

TEST(db_del_multiple) {
    FakeClock clock;
    Database db(clock);
    db.Set("a", "1");
    db.Set("b", "2");
    db.Set("c", "3");
    ASSERT_EQ(db.Del({"a", "b", "x"}), 2);
    ASSERT_FALSE(db.Exists("a"));
    ASSERT_FALSE(db.Exists("b"));
    ASSERT_TRUE(db.Exists("c"));
}

TEST(db_type_string) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "val");
    auto r = db.Type("key");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), "string");
}

TEST(db_mset_mget) {
    FakeClock clock;
    Database db(clock);
    db.MSet({{"a", "1"}, {"b", "2"}, {"c", "3"}});
    auto r = db.MGet({"a", "b", "x", "c"});
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value().size(), 4u);
    ASSERT_EQ(r.value()[0].value(), "1");
    ASSERT_EQ(r.value()[1].value(), "2");
    ASSERT_FALSE(r.value()[2].has_value());
    ASSERT_EQ(r.value()[3].value(), "3");
}

TEST(db_append_strlen) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "hel");
    auto appended = db.Append("key", "lo");
    ASSERT_TRUE(appended.ok());
    ASSERT_EQ(appended.value(), 5);
    ASSERT_EQ(db.Strlen("key").value(), 5);
}

TEST(db_incr_decr) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.Incr("counter").value(), 1);
    ASSERT_EQ(db.Incr("counter").value(), 2);
    ASSERT_EQ(db.Decr("counter").value(), 1);
}

TEST(db_incr_non_integer) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "not_a_number");
    auto r = db.Incr("key");
    ASSERT_FALSE(r.ok());
    ASSERT_EQ(r.error_code(), ErrorCode::kNotInteger);
}

TEST_MAIN()
