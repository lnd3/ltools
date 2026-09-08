#include <nn/fann/NNFannBase.h>

namespace l::nn::fann {

    std::optional<TargetVec> NNFannBase::infer(const InputVec& input) {
        if (input.size() != input_size_) return std::nullopt;

        float* output = fann_run(net_, const_cast<float*>(input.data()));
        return TargetVec(output, output + output_size_);
    }

    void NNFannBase::forceTrain(const TrainingExample& ex) {
        if (ex.input.size() != input_size_ || ex.target.size() != output_size_) {
            std::cerr << "Input/target size mismatch in forceTrain\n";
            return;
        }
        fann_train(net_, const_cast<float*>(ex.input.data()), const_cast<float*>(ex.target.data()));
        // markTrained called by trainIfNeeded
    }

    void NNFannBase::loadModel(const std::string& path) {
        struct fann* loaded = fann_create_from_file(path.c_str());
        if (loaded) {
            fann_destroy(net_);
            net_ = nullptr;
        }
        else {
            std::cerr << "Failed to load FANN model from " << path << "\n";
        }
    }

    void NNFannBase::saveModel(const std::string& path) const {
        fann_save(net_, path.c_str());
    }

}
