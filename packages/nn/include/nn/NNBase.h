#pragma once

#include <nn/NNUtils.h>

#include <vector>

namespace l::nn::fann {

    class NNBase {
    public:
        NNBase() = default;
        NNBase(std::string name) : name_(std::move(name)) {}
        virtual ~NNBase() = default;

        // Perform inference given input vector; returns optional output
        virtual std::optional<TargetVec> infer(const InputVec& input) = 0;

        // Train if this time range has not been trained yet
        bool trainIfNeeded(const TrainingExample& example) {
            if (!hasTrainedOn(example.time)) {
                forceTrain(example);
                markTrained(example.time);
                return true;
            }
            return false;
        }

        // Force training, regardless of training history
        virtual void forceTrain(const TrainingExample& example) = 0;

        virtual void loadModel(const std::string& path) = 0;
        virtual void saveModel(const std::string& path) const = 0;

        bool hasTrainedOn(const TimeRange& range) const {
            for (const auto& trained : trained_ranges_) {
                if (trained.overlaps(range)) return true;
            }
            return false;
        }

        void markTrained(const TimeRange& range) {
            trained_ranges_.push_back(range);
            // Optional: merge overlapping ranges to keep list small (not implemented here)
        }

        const std::string& name() const { return name_; }

        protected:
        std::string name_;
        std::vector<TimeRange> trained_ranges_;
    };

}