#pragma once

#include <cstdint>
#include <chrono>

namespace miniredis {

// Abstract clock interface for testability
class Clock {
public:
    virtual ~Clock() = default;

    // Returns current time in milliseconds since epoch
    virtual int64_t NowMs() const = 0;
};

// Real system clock
class SystemClock : public Clock {
public:
    int64_t NowMs() const override {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }
};

// Fake clock for deterministic testing
class FakeClock : public Clock {
public:
    FakeClock() : now_ms_(0) {}

    int64_t NowMs() const override { return now_ms_; }

    void AdvanceMs(int64_t ms) { now_ms_ += ms; }
    void SetMs(int64_t ms) { now_ms_ = ms; }

private:
    int64_t now_ms_;
};

}  // namespace miniredis
