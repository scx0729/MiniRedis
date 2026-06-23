#include "test_framework.h"
#include <cstdio>
#include <cstring>

namespace test {

int TestRegistry::tests_run_ = 0;
int TestRegistry::tests_passed_ = 0;
int TestRegistry::tests_failed_ = 0;

TestRegistry& TestRegistry::Instance() {
    static TestRegistry instance;
    return instance;
}

void TestRegistry::AddTest(const char* name, TestFunc func) {
    tests_.push_back({name, func});
}

int TestRegistry::RunAll() {
    tests_run_ = 0;
    tests_passed_ = 0;
    tests_failed_ = 0;

    for (const auto& test : tests_) {
        ++tests_run_;
        printf("  [TEST] %s ... ", test.name);
        fflush(stdout);
        try {
            test.func();
            ++tests_passed_;
            printf("PASS\n");
        } catch (const TestFailure& e) {
            ++tests_failed_;
            printf("FAIL\n    %s:%d: %s\n", e.file, e.line, e.message);
        } catch (const std::exception& e) {
            ++tests_failed_;
            printf("ERROR\n    unexpected exception: %s\n", e.what());
        } catch (...) {
            ++tests_failed_;
            printf("ERROR\n    unknown exception\n");
        }
    }

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed_, tests_failed_, tests_run_);

    return tests_failed_ > 0 ? 1 : 0;
}

}  // namespace test
