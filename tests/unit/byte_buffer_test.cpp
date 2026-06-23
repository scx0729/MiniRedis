#include "test_framework.h"
#include "miniredis/server/byte_buffer.h"
#include <cstring>

using namespace miniredis;

TEST(empty_buffer) {
    ByteBuffer buf;
    ASSERT_EQ(buf.Available(), 0u);
    ASSERT_TRUE(buf.Peek().empty());
}

TEST(append_and_peek) {
    ByteBuffer buf;
    auto r = buf.Append("hello", 5);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(buf.Available(), 5u);
    ASSERT_EQ(buf.Peek(), "hello");
}

TEST(append_string_view) {
    ByteBuffer buf;
    std::string_view sv = "world";
    auto r = buf.Append(sv);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(buf.Peek(), "world");
}

TEST(consume_partial) {
    ByteBuffer buf;
    buf.Append("hello world", 11);
    auto r = buf.Consume(6);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(buf.Available(), 5u);
    ASSERT_EQ(buf.Peek(), "world");
}

TEST(consume_all) {
    ByteBuffer buf;
    buf.Append("test", 4);
    buf.Consume(4);
    ASSERT_EQ(buf.Available(), 0u);
    ASSERT_TRUE(buf.Peek().empty());
}

TEST(consume_beyond_available_fails) {
    ByteBuffer buf;
    buf.Append("abc", 3);
    auto r = buf.Consume(5);
    ASSERT_FALSE(r.ok());
}

TEST(multiple_appends) {
    ByteBuffer buf;
    buf.Append("he", 2);
    buf.Append("llo", 3);
    ASSERT_EQ(buf.Available(), 5u);
    ASSERT_EQ(buf.Peek(), "hello");
}

TEST(max_size_limit) {
    ByteBuffer buf(10);
    buf.Append("12345", 5);
    auto r = buf.Append("678901", 6);
    ASSERT_FALSE(r.ok());
    ASSERT_EQ(buf.Available(), 5u);
}

TEST(compact) {
    ByteBuffer buf;
    buf.Append("hello world", 11);
    buf.Consume(6);  // consume "hello "
    ASSERT_EQ(buf.Available(), 5u);
    buf.Compact();
    ASSERT_EQ(buf.Available(), 5u);
    ASSERT_EQ(buf.Peek(), "world");
}

TEST(clear) {
    ByteBuffer buf;
    buf.Append("data", 4);
    buf.Clear();
    ASSERT_EQ(buf.Available(), 0u);
}

TEST(binary_data_with_nulls) {
    ByteBuffer buf;
    char data[] = {'a', '\0', 'b', '\0', 'c'};
    buf.Append(data, 5);
    ASSERT_EQ(buf.Available(), 5u);
    auto peek = buf.Peek();
    ASSERT_EQ(peek.size(), 5u);
    ASSERT_EQ(peek[1], '\0');
    ASSERT_EQ(peek[3], '\0');
}

TEST(empty_string_append) {
    ByteBuffer buf;
    buf.Append("", 0);
    ASSERT_EQ(buf.Available(), 0u);
}

TEST(repeated_consume_append_cycle) {
    ByteBuffer buf;
    for (int i = 0; i < 100; ++i) {
        std::string s = "msg" + std::to_string(i);
        buf.Append(s.data(), s.size());
        ASSERT_EQ(buf.Available(), s.size());
        buf.Consume(s.size());
        ASSERT_EQ(buf.Available(), 0u);
    }
}

TEST_MAIN()
