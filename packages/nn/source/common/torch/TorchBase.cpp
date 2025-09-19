#include <nn/torch/TorchBase.h>

namespace l::nn::libtorch {

#ifdef LDEPS_USE_LIBTORCH
    struct TransformerNet : torch::nn::Module {
        // Define transformer as in previous messages
        torch::nn::Conv1d conv{ nullptr };
        torch::nn::Transformer transformer{ nullptr };
        torch::nn::Linear fc{ nullptr };
        TransformerNet(int window_size, int output_size) {
            conv = register_module("conv", torch::nn::Conv1d(torch::nn::Conv1dOptions(1, 16, 5).stride(1).padding(2)));
            transformer = register_module("transformer", torch::nn::Transformer(torch::nn::TransformerOptions(16, 8).d_model(16).nhead(8)));
            fc = register_module("fc", torch::nn::Linear(16 * window_size, output_size));
        }
        torch::Tensor forward(torch::Tensor x) {
            x = x.unsqueeze(1);
            x = torch::relu(conv->forward(x));
            x = transformer->forward(x, x);
            x = x.view({ x.size(0), -1 });
            return fc->forward(x);
        }
    };
#endif

    void Transformer::process(const std::vector<float>& window) {
#ifdef LDEPS_USE_LIBTORCH
        static TransformerNet model(window.size(), 10); // Predict 10 samples
        model.eval();
        torch::NoGradGuard no_grad;
        auto input = torch::tensor(window).reshape({ 1, -1 });
        auto output = model.forward(input);
        std::vector<float> result(output.data_ptr<float>(), output.data_ptr<float>() + 10);

        //LLOG(LogInfo) << "result[0]: " << result.at(0);
#else
        // Fallback: Simple moving average or skip
        std::cout << "Torch not available. Skipping transformer processing.\n";
#endif
    }
}
