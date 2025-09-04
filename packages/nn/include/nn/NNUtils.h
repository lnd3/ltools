#pragma once

#include <vector>

namespace l::nn::fann {

    using UnixTime = int32_t;  // Unix timestamp in seconds

    struct TimeRange {
        UnixTime start;  // inclusive
        UnixTime end;    // exclusive

        bool overlaps(const TimeRange& other) const {
            return start < other.end && end > other.start;
        }

        bool contains(UnixTime t) const {
            return t >= start && t < end;
        }

        int32_t duration() const { return end - start; }
    };

    // ---------- Data Types ----------

    using InputVec = std::vector<float>;
    using TargetVec = std::vector<float>;

    struct TrainingExample {
        TimeRange time;
        InputVec input;
        TargetVec target;
    };

}