#pragma once

#include <nn/NNUtils.h>

#include <vector>
#include <optional>
#include <assert.h>

namespace l::nn {

    /**
     * Collects timestamped inputs and allows querying a fixed-size window
     */
    class SlidingWindowBuffer {
    public:
        SlidingWindowBuffer(size_t window_size_seconds, size_t sample_rate_hz)
            : window_size_(window_size_seconds), sample_rate_(sample_rate_hz)
        {
            max_samples_ = window_size_ * sample_rate_;
        }

        // Add a new timestamped sample (assumes monotonically increasing time)
        void push(UnixTime timestamp, const InputVec& input) {
            assert(!input.empty());
            timestamps_.push_back(timestamp);
            inputs_.push_back(input);

            // Maintain max size
            while (!timestamps_.empty() && timestamps_.front() < timestamp - window_size_) {
                timestamps_.erase(timestamps_.begin());
                inputs_.erase(inputs_.begin());
            }
        }

        // Return concatenated input vector for current window if complete
        std::optional<InputVec> getWindow() const {
            if (timestamps_.empty()) return std::nullopt;

            UnixTime window_start = timestamps_.front();
            UnixTime window_end = timestamps_.back();

            // Ensure window covers the full requested duration
            if (window_end - window_start + 1 < static_cast<UnixTime>(window_size_)) {
                return std::nullopt;  // Not enough data yet
            }

            // Concatenate all input vectors into one big vector
            InputVec concatenated;
            for (const auto& sample : inputs_) {
                concatenated.insert(concatenated.end(), sample.begin(), sample.end());
            }
            return concatenated;
        }

        TimeRange currentTimeRange() const {
            if (timestamps_.empty()) return { 0,0 };
            return { timestamps_.front(), timestamps_.back() + 1 };  // +1 exclusive
        }

    private:
        size_t window_size_;       // seconds
        size_t sample_rate_;       // samples per second
        size_t max_samples_;

        std::vector<UnixTime> timestamps_;
        std::vector<InputVec> inputs_;
    };

}

