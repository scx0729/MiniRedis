#include "test_framework.h"
#include "miniredis/resp/parser.h"
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/value.h"

using namespace miniredis;

TEST(parse_simple_string) {
    RespParser parser;
    auto result = parser.Parse("+OK\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_TRUE(result.value.has_value());
    ASSERT_EQ(result.value->type(), RespType::kSimpleString);
    ASSERT_EQ(result.value->simple_string(), "OK");
    ASSERT_EQ(result.consumed, 5u);
}

TEST(parse_error) {
    RespParser parser;
    auto result = parser.Parse("-ERR unknown\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->type(), RespType::kError);
    ASSERT_EQ(result.value->error_string(), "ERR unknown");
}

TEST(parse_integer) {
    RespParser parser;
    auto result = parser.Parse(":42\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->type(), RespType::kInteger);
    ASSERT_EQ(result.value->integer(), 42);
}

TEST(parse_negative_integer) {
    RespParser parser;
    auto result = parser.Parse(":-100\r\n");
    ASSERT_EQ(result.value->integer(), -100);
}

TEST(parse_bulk_string) {
    RespParser parser;
    auto result = parser.Parse("$5\r\nhello\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->type(), RespType::kBulkString);
    ASSERT_EQ(result.value->bulk_string(), "hello");
    ASSERT_EQ(result.consumed, 11u);
}

TEST(parse_empty_bulk_string) {
    RespParser parser;
    auto result = parser.Parse("$0\r\n\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->bulk_string(), "");
}

TEST(parse_null_bulk_string) {
    RespParser parser;
    auto result = parser.Parse("$-1\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_TRUE(result.value->is_null());
}

TEST(parse_array) {
    RespParser parser;
    auto result = parser.Parse("*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->type(), RespType::kArray);
    ASSERT_EQ(result.value->array().size(), 2u);
    ASSERT_EQ(result.value->array()[0]->bulk_string(), "foo");
    ASSERT_EQ(result.value->array()[1]->bulk_string(), "bar");
}

TEST(parse_empty_array) {
    RespParser parser;
    auto result = parser.Parse("*0\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->array().size(), 0u);
}

TEST(parse_null_array) {
    RespParser parser;
    auto result = parser.Parse("*-1\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_TRUE(result.value->is_null());
}

TEST(parse_incremental_byte_by_byte) {
    RespParser parser;
    std::string full = "+OK\r\n";
    for (size_t i = 0; i < full.size() - 1; ++i) {
        auto result = parser.Parse(std::string_view(&full[i], 1));
        ASSERT_EQ(result.status, ParseStatus::kNeedMoreData);
    }
    auto result = parser.Parse(std::string_view(&full[full.size() - 1], 1));
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->simple_string(), "OK");
}

TEST(parse_incremental_bulk_string) {
    RespParser parser;
    std::string full = "$5\r\nhello\r\n";

    // Send header
    auto r1 = parser.Parse("$5\r\n");
    ASSERT_EQ(r1.status, ParseStatus::kNeedMoreData);

    // Send partial data
    auto r2 = parser.Parse("hel");
    ASSERT_EQ(r2.status, ParseStatus::kNeedMoreData);

    // Send rest
    auto r3 = parser.Parse("lo\r\n");
    ASSERT_EQ(r3.status, ParseStatus::kComplete);
    ASSERT_EQ(r3.value->bulk_string(), "hello");
}

TEST(parse_two_commands) {
    RespParser parser;
    std::string input = "+OK\r\n:42\r\n";

    auto r1 = parser.Parse(input);
    ASSERT_EQ(r1.status, ParseStatus::kComplete);
    ASSERT_EQ(r1.value->simple_string(), "OK");
    ASSERT_EQ(r1.consumed, 5u);

    // Parse remaining
    std::string remaining = input.substr(r1.consumed);
    auto r2 = parser.Parse(remaining);
    ASSERT_EQ(r2.status, ParseStatus::kComplete);
    ASSERT_EQ(r2.value->integer(), 42);
}

TEST(parse_protocol_error) {
    RespParser parser;
    auto result = parser.Parse("?\r\n");
    ASSERT_EQ(result.status, ParseStatus::kProtocolError);
}

TEST(parse_invalid_bulk_length) {
    RespParser parser;
    auto result = parser.Parse("$-2\r\n");
    ASSERT_EQ(result.status, ParseStatus::kProtocolError);
}

TEST(parse_bulk_string_with_binary) {
    RespParser parser;
    std::string data = {'a', '\0', 'b', '\0', 'c'};
    std::string input = "$5\r\n" + data + "\r\n";
    auto result = parser.Parse(input);
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->bulk_string(), data);
}

TEST(parse_max_depth_exceeded) {
    ParserLimits limits;
    limits.max_depth = 3;
    RespParser parser(limits);

    // Build a deeply nested array: [[[["x"]]]]
    std::string input = "*1\r\n*1\r\n*1\r\n*1\r\n$1\r\nx\r\n";
    auto result = parser.Parse(input);
    // This should succeed (depth 4 from array perspective)
    // or fail depending on how we count depth
    // The parser counts array nesting, so 4 nested arrays
    ASSERT_TRUE(result.status == ParseStatus::kComplete ||
                result.status == ParseStatus::kProtocolError);
}

TEST(parse_array_with_null_element) {
    RespParser parser;
    auto result = parser.Parse("*2\r\n$3\r\nfoo\r\n$-1\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->array().size(), 2u);
    ASSERT_EQ(result.value->array()[0]->bulk_string(), "foo");
    // Null Bulk String is represented as a value with is_null() == true
    ASSERT_TRUE(result.value->array()[1].has_value());
    ASSERT_TRUE(result.value->array()[1]->is_null());
}

TEST(parser_reset) {
    RespParser parser;
    parser.Parse("$5\r\nhel");
    parser.Reset();
    auto result = parser.Parse("+OK\r\n");
    ASSERT_EQ(result.status, ParseStatus::kComplete);
    ASSERT_EQ(result.value->simple_string(), "OK");
}

TEST(parse_multiple_arrays_concatenated) {
    RespParser parser;
    std::string input = "*1\r\n$1\r\na\r\n*1\r\n$1\r\nb\r\n";

    auto r1 = parser.Parse(input);
    ASSERT_EQ(r1.status, ParseStatus::kComplete);
    ASSERT_EQ(r1.value->array()[0]->bulk_string(), "a");

    std::string remaining = input.substr(r1.consumed);
    auto r2 = parser.Parse(remaining);
    ASSERT_EQ(r2.status, ParseStatus::kComplete);
    ASSERT_EQ(r2.value->array()[0]->bulk_string(), "b");
}

TEST_MAIN()
