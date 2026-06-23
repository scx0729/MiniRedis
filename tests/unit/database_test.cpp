#include "test_framework.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
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

TEST(db_del_nonexistent) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.Del("nokey"), 0);
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

TEST(db_exists) {
    FakeClock clock;
    Database db(clock);
    ASSERT_FALSE(db.Exists("key"));
    db.Set("key", "val");
    ASSERT_TRUE(db.Exists("key"));
}

TEST(db_type_string) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "val");
    auto r = db.Type("key");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), "string");
}

TEST(db_type_nonexistent) {
    FakeClock clock;
    Database db(clock);
    auto r = db.Type("nokey");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), "none");
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

TEST(db_append) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "hel");
    auto r = db.Append("key", "lo");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), 5);
    auto g = db.Get("key");
    ASSERT_EQ(g.value().value(), "hello");
}

TEST(db_append_new_key) {
    FakeClock clock;
    Database db(clock);
    auto r = db.Append("key", "new");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), 3);
}

TEST(db_strlen) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "hello");
    ASSERT_EQ(db.Strlen("key").value(), 5);
    ASSERT_EQ(db.Strlen("nokey").value(), 0);
}

TEST(db_incr) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.Incr("counter").value(), 1);
    ASSERT_EQ(db.Incr("counter").value(), 2);
    ASSERT_EQ(db.Incr("counter").value(), 3);
}

TEST(db_incr_non_integer) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "not_a_number");
    auto r = db.Incr("key");
    ASSERT_FALSE(r.ok());
    ASSERT_EQ(r.error_code(), ErrorCode::kNotInteger);
}

TEST(db_decr) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.Decr("counter").value(), -1);
    ASSERT_EQ(db.Decr("counter").value(), -2);
}

TEST(db_lpush_rpush) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.LPush("list", {"a"}).value(), 1);
    ASSERT_EQ(db.LPush("list", {"b"}).value(), 2);
    ASSERT_EQ(db.RPush("list", {"c"}).value(), 3);
    ASSERT_EQ(db.LLen("list").value(), 3);
}

TEST(db_lpop_rpop) {
    FakeClock clock;
    Database db(clock);
    db.RPush("list", {"a", "b", "c"});
    ASSERT_EQ(db.LPop("list").value().value(), "a");
    ASSERT_EQ(db.RPop("list").value().value(), "c");
    ASSERT_EQ(db.LLen("list").value(), 1);
}

TEST(db_lpop_removes_empty_key) {
    FakeClock clock;
    Database db(clock);
    db.RPush("list", {"only"});
    db.LPop("list");
    ASSERT_FALSE(db.Exists("list"));
}

TEST(db_lrange) {
    FakeClock clock;
    Database db(clock);
    db.RPush("list", {"a", "b", "c", "d"});
    auto r = db.LRange("list", 1, 2);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value().size(), 2u);
    ASSERT_EQ(r.value()[0], "b");
    ASSERT_EQ(r.value()[1], "c");
}

TEST(db_lrange_negative) {
    FakeClock clock;
    Database db(clock);
    db.RPush("list", {"a", "b", "c"});
    auto r = db.LRange("list", -2, -1);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value().size(), 2u);
    ASSERT_EQ(r.value()[0], "b");
    ASSERT_EQ(r.value()[1], "c");
}

TEST(db_lrange_out_of_range) {
    FakeClock clock;
    Database db(clock);
    db.RPush("list", {"a", "b"});
    auto r = db.LRange("list", 0, 100);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value().size(), 2u);
}

TEST(db_lrange_nonexistent) {
    FakeClock clock;
    Database db(clock);
    auto r = db.LRange("nokey", 0, -1);
    ASSERT_TRUE(r.ok());
    ASSERT_TRUE(r.value().empty());
}

TEST(db_hset_hget) {
    FakeClock clock;
    Database db(clock);
    db.HSet("user", {{"name", "alice"}, {"age", "20"}});
    ASSERT_EQ(db.HGet("user", "name").value().value(), "alice");
    ASSERT_EQ(db.HGet("user", "age").value().value(), "20");
    ASSERT_FALSE(db.HGet("user", "missing").value().has_value());
}

TEST(db_hset_update) {
    FakeClock clock;
    Database db(clock);
    db.HSet("user", {{"name", "alice"}});
    db.HSet("user", {{"name", "bob"}});
    ASSERT_EQ(db.HGet("user", "name").value().value(), "bob");
}

TEST(db_hdel) {
    FakeClock clock;
    Database db(clock);
    db.HSet("user", {{"a", "1"}, {"b", "2"}, {"c", "3"}});
    ASSERT_EQ(db.HDel("user", {"a", "x"}).value(), 1);
    ASSERT_EQ(db.HLen("user").value(), 2);
}

TEST(db_hdel_removes_empty_key) {
    FakeClock clock;
    Database db(clock);
    db.HSet("user", {{"only", "field"}});
    db.HDel("user", {"only"});
    ASSERT_FALSE(db.Exists("user"));
}

TEST(db_hlen) {
    FakeClock clock;
    Database db(clock);
    ASSERT_EQ(db.HLen("nokey").value(), 0);
    db.HSet("user", {{"a", "1"}, {"b", "2"}});
    ASSERT_EQ(db.HLen("user").value(), 2);
}

TEST(db_hgetall_sorted) {
    FakeClock clock;
    Database db(clock);
    db.HSet("user", {{"name", "alice"}, {"age", "20"}, {"city", "NYC"}});
    auto r = db.HGetAll("user");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value().size(), 6u);
    // Should be sorted by field name
    ASSERT_EQ(r.value()[0], "age");
    ASSERT_EQ(r.value()[1], "20");
    ASSERT_EQ(r.value()[2], "city");
    ASSERT_EQ(r.value()[3], "NYC");
    ASSERT_EQ(r.value()[4], "name");
    ASSERT_EQ(r.value()[5], "alice");
}

TEST(db_wrong_type_string_on_list) {
    FakeClock clock;
    Database db(clock);
    db.RPush("list", {"a"});
    auto r = db.Get("list");
    ASSERT_FALSE(r.ok());
    ASSERT_EQ(r.error_code(), ErrorCode::kWrongType);
}

TEST(db_wrong_type_list_on_string) {
    FakeClock clock;
    Database db(clock);
    db.Set("key", "val");
    auto r = db.LLen("key");
    ASSERT_FALSE(r.ok());
    ASSERT_EQ(r.error_code(), ErrorCode::kWrongType);
}

TEST(db_set_overwrites_any_type) {
    FakeClock clock;
    Database db(clock);
    db.RPush("key", {"a"});
    db.Set("key", "string_val");
    auto r = db.Get("key");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value().value(), "string_val");
}

TEST_MAIN()
