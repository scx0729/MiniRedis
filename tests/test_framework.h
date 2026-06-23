#pragma once

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

namespace test {

struct TestFailure {
    const char* file;
    int line;
    const char* message;
};

class TestRegistry {
public:
    using TestFunc = std::function<void()>;

    static TestRegistry& Instance();

    void AddTest(const char* name, TestFunc func);
    int RunAll();

private:
    struct TestEntry {
        const char* name;
        TestFunc func;
    };
    std::vector<TestEntry> tests_;
    static int tests_run_;
    static int tests_passed_;
    static int tests_failed_;
};

}  // namespace test

#define TEST(name)                                                      \
    static void test_##name();                                          \
    namespace {                                                         \
    struct Register_##name {                                            \
        Register_##name() {                                             \
            test::TestRegistry::Instance().AddTest(#name, test_##name); \
        }                                                               \
    } register_##name;                                                  \
    }                                                                   \
    static void test_##name()

#define ASSERT_TRUE(expr)                                               \
    do {                                                                \
        if (!(expr)) {                                                  \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_TRUE failed: " #expr};      \
        }                                                               \
    } while (0)

#define ASSERT_FALSE(expr)                                              \
    do {                                                                \
        if ((expr)) {                                                   \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_FALSE failed: " #expr};     \
        }                                                               \
    } while (0)

#define ASSERT_EQ(a, b)                                                 \
    do {                                                                \
        if ((a) != (b)) {                                               \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_EQ failed: " #a " != " #b}; \
        }                                                               \
    } while (0)

#define ASSERT_NE(a, b)                                                 \
    do {                                                                \
        if ((a) == (b)) {                                               \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_NE failed: " #a " == " #b}; \
        }                                                               \
    } while (0)

#define ASSERT_GT(a, b)                                                 \
    do {                                                                \
        if ((a) <= (b)) {                                               \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_GT failed: " #a " <= " #b}; \
        }                                                               \
    } while (0)

#define ASSERT_GE(a, b)                                                 \
    do {                                                                \
        if ((a) < (b)) {                                                \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_GE failed: " #a " < " #b};  \
        }                                                               \
    } while (0)

#define ASSERT_LT(a, b)                                                 \
    do {                                                                \
        if ((a) >= (b)) {                                               \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_LT failed: " #a " >= " #b}; \
        }                                                               \
    } while (0)

#define ASSERT_LE(a, b)                                                 \
    do {                                                                \
        if ((a) > (b)) {                                                \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_LE failed: " #a " > " #b};  \
        }                                                               \
    } while (0)

#define ASSERT_STREQ(a, b)                                              \
    do {                                                                \
        if (std::string(a) != std::string(b)) {                        \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                                    "ASSERT_STREQ failed"};            \
        }                                                               \
    } while (0)

#define ASSERT_THROW(expr, exception_type)                              \
    do {                                                                \
        bool caught = false;                                            \
        try { expr; } catch (const exception_type&) { caught = true; }  \
        catch (...) {}                                                  \
        if (!caught) {                                                  \
            throw test::TestFailure{__FILE__, __LINE__,                 \
                    "ASSERT_THROW failed: expected " #exception_type};  \
        }                                                               \
    } while (0)

#define RUN_ALL_TESTS() test::TestRegistry::Instance().RunAll()

// Main macro for test executables
#define TEST_MAIN()                                                     \
    int main() { return RUN_ALL_TESTS(); }
