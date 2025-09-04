#pragma once

#include <vector>
#include <unordered_set>
#include <string>
#include <memory>
#include <optional>
#include <iostream>
#include <algorithm>
#include <cassert>

#include <nn/NNBase.h>

#define FANN_NO_DLL
#include <floatfann.h>

namespace l::nn::fann {

    class FannNNBase : public NNBase {
    public:
        FannNNBase() = default;
        FannNNBase(std::string name, unsigned inputSize, unsigned outputSize)
            : NNBase(std::move(name)), input_size_(inputSize), output_size_(outputSize)
        {
            
            net_ = fann_create_standard(3, inputSize, 16, outputSize);
            fann_set_training_algorithm(net_, FANN_TRAIN_RPROP);
            fann_set_learning_rate(net_, 0.01f);
        }

        ~FannNNBase() override = default;

        std::optional<TargetVec> infer(const InputVec& input) override;
        void forceTrain(const TrainingExample& ex) override;
        void loadModel(const std::string& path) override;
        void saveModel(const std::string& path) const override;

    private:
        struct fann* net_ = nullptr;
        unsigned input_size_;
        unsigned output_size_;
    };


}