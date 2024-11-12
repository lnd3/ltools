#include <unordered_map>
#include <map>

#include "testing/Timer.h"

namespace l {
namespace testing {

    std::string& get_current_test_group() {
        static std::string group;
        return group;
    }

    Timer::Timer(std::function<void(uint64_t)> result) : mResult(std::move(result)) {
        Start();
    }

    Timer::~Timer() {
        Stop();
    }

    void Timer::Start() {
        mStartTime = std::chrono::steady_clock::now();
    }

    void Timer::Stop() {
        auto endTime = std::chrono::steady_clock::now();
        auto Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - mStartTime);
        if (mResult) {
            mResult(Duration.count());
        }
    }

    std::map<std::string, TimeMeasure>& get_time_measures(std::string_view groupName) {
        static std::map<std::string, std::map<std::string, TimeMeasure>> sTimers;

        auto it = sTimers.find(groupName.data());
        if (it != sTimers.end()) {
            return it->second;
        }
        sTimers.emplace(groupName, std::map<std::string, TimeMeasure>());
        it = sTimers.find(groupName.data());
        return it->second;
    }

    TimeMeasure& get_time_measure(std::string_view groupName, std::string_view id) {
        auto& timers = get_time_measures(groupName);

        auto it = timers.find(id.data());
        if (it != timers.end()) {
            return it->second;
        }
        timers.emplace(id, TimeMeasure());
        it = timers.find(id.data());
        return it->second;
    }

    PerformanceTimer::PerformanceTimer(std::string_view group, std::string_view id) : mGroup(group), mId(id), mTimer([&](uint64_t nanoseconds) {
        auto& measure = get_time_measure(mGroup, mId);

        measure.mSeconds += static_cast<double>(nanoseconds) / 1000000000.0;
        measure.mCount++;

        }) {}

}
}
