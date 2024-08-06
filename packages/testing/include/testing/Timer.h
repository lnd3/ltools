#pragma once

#include <functional>
#include <chrono>
#include <memory>

#include "logging/Macro.h"

namespace l::testing {

    std::string& get_current_test_group();

    class Timer {
    public:
        Timer(std::function<void(uint64_t)> result);
        ~Timer();
        void Start();
        void Stop();
    protected:
        std::function<void(uint64_t)> mResult;

        std::chrono::steady_clock::time_point mStartTime;
    };

    struct TimeMeasure {
        double mSeconds = 0;
        uint64_t mCount = 0;
    };

    class PerformanceTimer {
    public:
        PerformanceTimer(std::string_view group, std::string_view id);
    protected:
        std::string mGroup;
        std::string mId;
        l::testing::Timer mTimer;
    };

    std::map<std::string, TimeMeasure>& get_time_measures(std::string_view groupName);
    TimeMeasure& get_time_measure(std::string_view groupName, std::string_view id);
}

#define PERF_TIMER(name) auto UNIQUE(PerfTimer) = std::make_unique<l::testing::PerformanceTimer>(l::testing::get_current_test_group(), name)

#define PERF_TIMER_RESULT(name) l::testing::get_time_measure(l::testing::get_current_test_group(), name)
