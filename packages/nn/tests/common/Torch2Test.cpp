#include "testing/Test.h"
#include "logging/Log.h"

#include <nn/torch/TorchBase.h>

TEST(Torch2, TorchTensorBasic) {
    torch::Tensor tensor = torch::eye(3);
    std::cout << tensor << std::endl;
    return 0;
}

struct Net1Impl : torch::nn::Module {
    Net1Impl(int64_t N, int64_t M) {
        linear = register_module("linear", torch::nn::Linear(N, M));
        another_bias = register_parameter("b", torch::randn(M));
    }
    torch::Tensor forward(torch::Tensor input) {
        return linear(input) + another_bias;
    }
    torch::nn::Linear linear = nullptr;
    torch::Tensor another_bias;
};
TORCH_MODULE(Net1);

TEST(Torch2, TorchNetBasic) {
    return 0;
    Net1 net(4, 5);

    for (const auto& p : net->parameters()) {
        std::cout << p << std::endl;
    }
    return 0;
}


// Define a new Module.
struct Net2Impl : torch::nn::Module {
    Net2Impl() {
        // Construct and register two Linear submodules.
        fc1 = register_module("fc1", torch::nn::Linear(784, 64));
        fc2 = register_module("fc2", torch::nn::Linear(64, 32));
        fc3 = register_module("fc3", torch::nn::Linear(32, 10));
        //fc1->to(torch::kCUDA);
        //fc2->to(torch::kCUDA);
        //fc3->to(torch::kCUDA);
    }

    TORCH_API torch::serialize::OutputArchive& operator<<(
        torch::serialize::OutputArchive& archive) {
        archive << fc1;
        archive << fc2;
        archive << fc3;
    }

    /// Deserializes a `Module` from an `InputArchive`.
    TORCH_API torch::serialize::InputArchive& operator>>(
        torch::serialize::InputArchive& archive) {
        archive >> fc3;
        archive >> fc2;
        archive >> fc1;
    }

    // Implement the Net's algorithm.
    torch::Tensor forward(torch::Tensor x) {
        // Use one of many tensor manipulation functions.
        x = torch::relu(fc1->forward(x.reshape({ x.size(0), 784 })));
        x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
        x = torch::relu(fc2->forward(x));
        x = torch::log_softmax(fc3->forward(x), /*dim=*/1);
        return x;
    }

    // Use one of many "standard library" modules.
    torch::nn::Linear fc1{ nullptr }, fc2{ nullptr }, fc3{ nullptr };
};
TORCH_MODULE(Net2);

TEST(Torch2, TorchAdvancedExample) {
    return 0;
    // Create a new Net.
    auto net = std::make_shared<Net2>();

    // Create a multi-threaded data loader for the MNIST dataset.
    auto data_loader = torch::data::make_data_loader(
        torch::data::datasets::MNIST("./data").map(
            torch::data::transforms::Stack<>()),
        /*batch_size=*/64);

    // Instantiate an SGD optimization algorithm to update our Net's parameters.
    torch::optim::SGD optimizer(net->get()->parameters(), /*lr=*/0.01);

    for (size_t epoch = 1; epoch <= 10; ++epoch) {
        size_t batch_index = 0;
        // Iterate the data loader to yield batches from the dataset.
        for (auto& batch : *data_loader) {
            // Reset gradients.
            optimizer.zero_grad();
            // Execute the model on the input data.
            torch::Tensor prediction = net->get()->forward(batch.data);
            // Compute a loss value to judge the prediction of our model.
            torch::Tensor loss = torch::nll_loss(prediction, batch.target);
            // Compute gradients of the loss w.r.t. the parameters of our model.
            loss.backward();
            // Update the parameters based on the calculated gradients.
            optimizer.step();
            // Output the loss and checkpoint every 100 batches.
            if (++batch_index % 100 == 0) {
                std::cout << "Epoch: " << epoch << " | Batch: " << batch_index
                    << " | Loss: " << loss.item<float>() << std::endl;
                // Serialize your model periodically as a checkpoint.
                //torch::save(net, "net.pt");
            }
        }
    }
    return 0;
}
