#include "testing/Test.h"
#include "logging/Log.h"

#ifdef HAS_LIBTORCH
#include <nn/torch/TorchBase.h>

#include <vector>
#include <deque>
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>

using namespace torch::indexing;

// === 1. Multi-Octave Sliding Window Trainer ===
struct OctaveTrainer : torch::nn::Module {
    struct Window {
        std::deque<torch::Tensor> X, y;
        int size;

        Window(int s) : size(s) {}

        void add(torch::Tensor x, torch::Tensor target) {
            X.push_back(x);
            y.push_back(target);
            if ((int)X.size() > size) {
                X.pop_front();
                y.pop_front();
            }
        }
    };

    std::vector<Window> windows;
    torch::nn::Linear model{ nullptr };
    std::unique_ptr<torch::optim::Adam> optimizer;
    torch::Device device;

    OctaveTrainer(int input_dim) : device(torch::kCPU) {
        // Check for CUDA availability
        if (torch::cuda::is_available()) {
            std::cout << "CUDA is available! Training on GPU." << std::endl;
            device = torch::Device(torch::kCUDA);
        }
        else {
            std::cout << "CUDA is not available. Training on CPU." << std::endl;
        }

        // Initialize windows correctly
        windows.emplace_back(30);
        windows.emplace_back(90);
        windows.emplace_back(365);

        // Create the model properly using register_module
        model = register_module("linear", torch::nn::Linear(input_dim, 2));  // mean + logvar

        // Move model to device
        model->to(device);

        // Initialize weights and biases after registration
        torch::nn::init::normal_(model->weight, 0.0, 0.1);
        torch::nn::init::constant_(model->bias, 0.0);

        optimizer = std::make_unique<torch::optim::Adam>(torch::optim::Adam(model->parameters(), torch::optim::AdamOptions(1e-3)));
    }

    void train_step(torch::Tensor x, torch::Tensor target) {
        // Move tensors to device
        x = x.to(device);
        target = target.to(device);

        model->train();
        optimizer->zero_grad();
        auto out = model->forward(x);
        auto mean = out.slice(1, 0, 1);
        auto logvar = out.slice(1, 1, 2);
        auto loss = ((target - mean).square() / logvar.exp() + logvar).mean();
        loss.backward();
        optimizer->step();

        // Update all windows
        for (auto& w : windows) {
            w.add(x.detach(), target.detach());
        }
    }

    torch::Tensor predict(torch::Tensor x) {
        model->eval();
        torch::NoGradGuard no_grad;
        x = x.to(device);
        return model->forward(x);
    }
};

// === 2. OctaveConformal (for path weighting) ===
struct OctaveConformal {
    std::vector<std::deque<double>> residuals;
    std::vector<double> weights = { 1.0, 0.7, 0.4 };  // 30d, 90d, 365d
    torch::Device device;

    OctaveConformal() : device(torch::kCPU) {
        if (torch::cuda::is_available()) {
            device = torch::Device(torch::kCUDA);
        }
    }

    void update(torch::nn::Linear model, const std::vector<OctaveTrainer::Window>& windows) {
        residuals.clear();
        for (size_t i = 0; i < windows.size(); ++i) {
            std::deque<double> res;
            auto& w = windows[i];
            for (size_t j = 0; j < w.X.size(); ++j) {
                // Move tensor to device before forward pass
                auto x = w.X[j].to(device);
                auto out = model->forward(x);
                double mean = out[0][0].item<double>();
                double true_y = w.y[j].item<double>();
                res.push_back(std::abs(true_y - mean));
            }
            residuals.push_back(res);
        }
    }

    double p_value(double residual) {
        double count = 0, total = 0;
        for (size_t i = 0; i < residuals.size(); ++i) {
            double w = weights[i];
            for (double r : residuals[i]) {
                if (r <= residual) count += w;
                total += w;
            }
        }
        return (count + 1.0) / (total + 1.0);
    }
};

// === 3. Monte Carlo Path Generator ===
std::vector<std::vector<double>> monte_carlo_forecast(
    torch::nn::Linear model,
    torch::Tensor current_x,
    int steps,
    int paths,
    OctaveConformal& conformal,
    std::mt19937& rng,
    const torch::Device& device
) {
    std::vector<std::vector<double>> all_paths(paths, std::vector<double>(steps));
    std::normal_distribution<double> noise(0, 1);

    for (int p = 0; p < paths; ++p) {
        torch::Tensor x = current_x.clone().to(device);
        for (int t = 0; t < steps; ++t) {
            model->eval();
            auto out = model->forward(x);
            double mean = out[0][0].item<double>();
            double logvar = out[0][1].item<double>();
            double sigma = std::exp(0.5 * logvar);
            double y = mean + sigma * noise(rng);
            all_paths[p][t] = y;

            // Update input for next step (autoregressive)
            auto new_val = torch::tensor({ {y} }, torch::TensorOptions().device(device));
            x = torch::cat({ x.narrow(1, 1, x.size(1) - 1), new_val }, 1);
        }
    }
    return all_paths;
}


TEST(Torch, ChecForCuda) {
    std::cout << "=== CUDA Diagnostics ===" << std::endl;
    std::cout << "PyTorch version: " << TORCH_VERSION_MAJOR << "." 
              << TORCH_VERSION_MINOR << "." << TORCH_VERSION_PATCH << std::endl;
    std::cout << "CUDA available: " << torch::cuda::is_available() << std::endl;
    std::cout << "cuDNN available: " << torch::cuda::cudnn_is_available() << std::endl;
    
    if (torch::cuda::is_available()) {
        std::cout << "CUDA devices: " << torch::cuda::device_count() << std::endl;
        for (int i = 0; i < torch::cuda::device_count(); ++i) {
            std::cout << "CUDA device " << i << std::endl;
        }
    } else {
        std::cout << "CUDA is not available. Common causes:" << std::endl;
        std::cout << "1. LibTorch was built without CUDA support" << std::endl;
        std::cout << "2. CUDA runtime not installed or incompatible version" << std::endl;
        std::cout << "3. GPU driver issues" << std::endl;
        std::cout << "4. Missing environment variables" << std::endl;
    }
    std::cout << "========================" << std::endl;

    return 0;
}

TEST(Torch, Basic) {

    try {
        // Check CUDA availability
        std::cout << "PyTorch version: " << TORCH_VERSION_MAJOR << "."
            << TORCH_VERSION_MINOR << "." << TORCH_VERSION_PATCH << std::endl;
        std::cout << "CUDA available: " << torch::cuda::is_available() << std::endl;

        if (torch::cuda::is_available()) {
            std::cout << "CUDA devices: " << torch::cuda::device_count() << std::endl;
            for (int i = 0; i < torch::cuda::device_count(); ++i) {
                std::cout << "CUDA device " << i << std::endl;
            }
        }

        std::mt19937 rng(42);
        std::normal_distribution<double> noise(0, 1);
        int input_dim = 5;  // [lag1, lag2, bed, bath, trend]
        int steps_ahead = 30;
        int paths = 1000;

        OctaveTrainer trainer(input_dim);
        OctaveConformal conformal;

        // Simulate time series
        std::vector<double> true_series;
        torch::Tensor x = torch::zeros({ 1, input_dim });

        // Set device for initial tensor
        if (torch::cuda::is_available()) {
            x = x.to(torch::kCUDA);
        }

        for (int t = 0; t < 600; ++t) {
            double trend = 200 + 50 * std::sin(t / 365.0 * M_PI);
            double shock = (t == 300) ? -80 : (t == 500) ? +60 : 0;
            double price = trend + shock + 15 * noise(rng);
            true_series.push_back(price);

            torch::Tensor target = torch::tensor({ {price} });
            // Move target to the same device as model
            if (torch::cuda::is_available()) {
                target = target.to(torch::kCUDA);
            }
            trainer.train_step(x, target);

            if (t >= 365) {
                conformal.update(trainer.model, trainer.windows);
                if (t == 500) {
                    std::cout << "FORECASTING 30 STEPS AHEAD FROM t=" << t << "\n";
                    auto future_paths = monte_carlo_forecast(
                        trainer.model, x, steps_ahead, paths, conformal, rng,
                        torch::cuda::is_available() ? torch::kCUDA : torch::kCPU
                    );

                    // Weigh paths with conformal p-value
                    std::vector<double> weights(paths);
                    double sum_w = 0;
                    for (int p = 0; p < paths; ++p) {
                        double path_residual = 0;
                        for (int s = 0; s < std::min(5, steps_ahead); ++s)
                            path_residual += std::abs(future_paths[p][s] - future_paths[p][0]);
                        path_residual /= 5;
                        weights[p] = conformal.p_value(path_residual);
                        sum_w += weights[p];
                    }
                    for (auto& w : weights) w /= sum_w;

                    // Compute 90% weighted prediction band
                    std::vector<std::vector<double>> sorted(steps_ahead);
                    for (int s = 0; s < steps_ahead; ++s) {
                        std::vector<std::pair<double, double>> vals;
                        for (int p = 0; p < paths; ++p)
                            vals.emplace_back(future_paths[p][s], weights[p]);
                        std::sort(vals.begin(), vals.end());
                        double cum = 0;
                        for (auto& [v, w] : vals) {
                            cum += w;
                            if (cum >= 0.05) { sorted[s].push_back(v); break; }
                        }
                        cum = 0;
                        for (auto it = vals.rbegin(); it != vals.rend(); ++it) {
                            cum += it->second;
                            if (cum >= 0.05) { sorted[s].push_back(it->first); break; }
                        }
                    }

                    // Output
                    std::ofstream out("forecast.csv");
                    out << "step,true,lower,upper,mean\n";
                    for (int s = 0; s < steps_ahead; ++s) {
                        double mean = 0;
                        for (int p = 0; p < paths; ++p) mean += future_paths[p][s] * weights[p];
                        double true_val = (t + s < true_series.size()) ? true_series[t + s] : NAN;
                        out << s << "," << true_val << "," << sorted[s][0] << "," << sorted[s][1] << "," << mean << "\n";
                        std::cout << "t+" << std::setw(2) << s
                            << " | True: $" << std::fixed << std::setprecision(1) << true_val
                            << "k | Band: [$" << sorted[s][0] << "k, $" << sorted[s][1] << "k]\n";
                    }
                    std::cout << "\nforecast.csv written.\n";
                    break;
                }
            }

            // Shift input window
            x = torch::cat({ x.narrow(1, 1, input_dim - 1), target }, 1);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }


    return 0;

}

#endif

