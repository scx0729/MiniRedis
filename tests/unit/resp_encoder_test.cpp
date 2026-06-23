#include "test_framework.h"
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/value.h"

using namespace miniredis;

TEST(encode_simple_string) {
    auto encoded = RespEncoder::Encode(RespValue::SimpleString("OK"));
    ASSERT_EQ(encoded, "+OK\r\n");
}

TEST(encode_error) {
    auto encoded = RespEncoder::Encode(RespValue::Error("ERR something"));
    ASSERT_EQ(encoded, "-ERR something\r\n");
}

TEST(encode_integer_positive) {
    auto encoded = RespEncoder::Encode(RespValue::Integer(42));
    ASSERT_EQ(encoded, ":42\r\n");
}

TEST(encode_integer_negative) {
    auto encoded = RespEncoder::Encode(RespValue::Integer(-1));
    ASSERT_EQ(encoded, ":-1\r\n");
}

TEST(encode_integer_zero) {
    auto encoded = RespEncoder::Encode(RespValue::Integer(0));
    ASSERT_EQ(encoded, ":0\r\n");
}

TEST(encode_bulk_string) {
    auto encoded = RespEncoder::Encode(RespValue::BulkString("hello"));
    ASSERT_EQ(encoded, "$5\r\nhello\r\n");
}

TEST(encode_empty_bulk_string) {
    auto encoded = RespEncoder::Encode(RespValue::BulkString(""));
    ASSERT_EQ(encoded, "$0\r\n\r\n");
}

TEST(encode_null_bulk_string) {
    auto encoded = RespEncoder::Encode(RespValue::NullBulkString());
    ASSERT_EQ(encoded, "$-1\r\n");
}

TEST(encode_bulk_string_with_crlf) {
    auto encoded = RespEncoder::Encode(RespValue::BulkString("line1\r\nline2"));
    ASSERT_EQ(encoded, "$12\r\nline1\r\nline2\r\n");
}

TEST(encode_bulk_string_with_null_bytes) {
    std::string s = {'a', '\0', 'b'};
    auto encoded = RespEncoder::Encode(RespValue::BulkString(s));
    std::string expected = std::string("$3\r\n") + std::string("a\0b", 3) + std::string("\r\n");
    ASSERT_EQ(encoded.size(), expected.size());
    ASSERT_TRUE(encoded == expected);
}

TEST(encode_array) {
    RespValue::ArrayT arr;
    arr.push_back(RespValue::BulkString("foo"));
    arr.push_back(RespValue::BulkString("bar"));
    auto encoded = RespEncoder::Encode(RespValue::MakeArray(std::move(arr)));
    ASSERT_EQ(encoded, "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n");
}

TEST(encode_empty_array) {
    auto encoded = RespEncoder::Encode(RespValue::MakeArray({}));
    ASSERT_EQ(encoded, "*0\r\n");
}

TEST(encode_null_array) {
    auto encoded = RespEncoder::Encode(RespValue::NullArray());
    ASSERT_EQ(encoded, "*-1\r\n");
}

TEST(encode_array_with_null_element) {
    RespValue::ArrayT arr;
    arr.push_back(RespValue::BulkString("hello"));
    arr.push_back(std::nullopt);
    auto encoded = RespEncoder::Encode(RespValue::MakeArray(std::move(arr)));
    ASSERT_EQ(encoded, "*2\r\n$5\r\nhello\r\n$-1\r\n");
}

TEST(encode_optional_with_value) {
    std::optional<RespValue> opt = RespValue::BulkString("test");
    auto encoded = RespEncoder::Encode(opt);
    ASSERT_EQ(encoded, "$4\r\ntest\r\n");
}

TEST(encode_optional_null) {
    std::optional<RespValue> opt;
    auto encoded = RespEncoder::Encode(opt);
    ASSERT_EQ(encoded, "$-1\r\n");
}

TEST(encode_nested_array) {
    RespValue::ArrayT inner;
    inner.push_back(RespValue::BulkString("a"));
    inner.push_back(RespValue::BulkString("b"));
    RespValue::ArrayT outer;
    outer.push_back(RespValue::MakeArray(std::move(inner)));
    outer.push_back(RespValue::Integer(42));
    auto encoded = RespEncoder::Encode(RespValue::MakeArray(std::move(outer)));
    ASSERT_EQ(encoded, "*2\r\n*2\r\n$1\r\na\r\n$1\r\nb\r\n:42\r\n");
}

TEST_MAIN()
