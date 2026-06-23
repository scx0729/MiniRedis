#include "test_framework.h"
#include "miniredis/base/result.h"

using namespace miniredis;

TEST(result_ok_int) {
    Result<int> r = 42;
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), 42);
}

TEST(result_err_int) {
    Result<int> r = Error(ErrorCode::kWrongType);
    ASSERT_FALSE(r.ok());
    ASSERT_EQ(r.error_code(), ErrorCode::kWrongType);
}

TEST(result_ok_string) {
    Result<std::string> r = std::string("hello");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(r.value(), "hello");
}

TEST(result_void_ok) {
    Result<void> r;
    ASSERT_TRUE(r.ok());
}

TEST(result_void_err) {
    Result<void> r = Error(ErrorCode::kIoError);
    ASSERT_FALSE(r.ok());
}

TEST(result_move_value) {
    Result<std::string> r = std::string("data");
    std::string moved = r.move_value();
    ASSERT_EQ(moved, "data");
}

TEST(result_bool_conversion) {
    Result<int> ok = 1;
    Result<int> err = Error(ErrorCode::kProtocolError);
    ASSERT_TRUE(static_cast<bool>(ok));
    ASSERT_FALSE(static_cast<bool>(err));
}

TEST_MAIN()
