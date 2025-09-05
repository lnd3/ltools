#include <nn/tinydnn/TrendReversalPredictor.h>

#include <nn/tinydnn/flatten_layer.h>

#include <fstream>
#include <sstream>
#include <chrono>

namespace l::nn::tinydnn {








    void TrendReversalPredictor::normalize(tiny_dnn::vec_t& data, float min, float max) {
        if (max == min) return;
        for (auto& val : data) val = (val - min) / (max - min);
    }

    class bce_loss {
    public:
        static tiny_dnn::float_t f(const tiny_dnn::vec_t& y, const tiny_dnn::vec_t& t) {
            if (y.size() != 1 || t.size() != 1) {
                throw std::runtime_error("bce_loss::f expects single output and target, got y.size: " +
                    std::to_string(y.size()) + ", t.size: " + std::to_string(t.size()));
            }
            tiny_dnn::float_t y_val = y[0];
            tiny_dnn::float_t t_val = t[0];
            if (t_val != 0.0f && t_val != 1.0f) {
                throw std::runtime_error("bce_loss::f expects binary target (0 or 1), got: " + std::to_string(t_val));
            }
            if (y_val < 0.0f || y_val > 1.0f) {
                throw std::runtime_error("bce_loss::f expects y in [0,1], got: " + std::to_string(y_val));
            }
            return -t_val * log(y_val + 1e-10) - (1 - t_val) * log(1 - y_val + 1e-10);
        }
        static tiny_dnn::vec_t df(const tiny_dnn::vec_t& y, const tiny_dnn::vec_t& t) {
            if (y.size() != 1 || t.size() != 1) {
                throw std::runtime_error("bce_loss::df expects single output and target, got y.size: " +
                    std::to_string(y.size()) + ", t.size: " + std::to_string(t.size()));
            }
            tiny_dnn::float_t y_val = y[0];
            tiny_dnn::float_t t_val = t[0];
            if (t_val != 0.0f && t_val != 1.0f) {
                throw std::runtime_error("bce_loss::df expects binary target (0 or 1), got: " + std::to_string(t_val));
            }
            if (y_val < 0.0f || y_val > 1.0f) {
                throw std::runtime_error("bce_loss::df expects y in [0,1], got: " + std::to_string(y_val));
            }
            tiny_dnn::float_t grad = (y_val - t_val) / ((y_val + 1e-10) * (1 - y_val + 1e-10));
            return { grad };
        }
    };

    TrendReversalPredictor::TrendReversalPredictor(size_t seq_len_, size_t ts_features_, size_t ext_features_)
        : seq_len(seq_len_), ts_features(ts_features_), ext_features(ext_features_),
        ts_buffer(seq_len* ts_features, 0.0f), ext_buffer(ext_features, 0.0f) {
        using namespace tiny_dnn;
        // Compute convolutional output size
        size_t kernel_size = 3, stride = 1, filters = 8;
        conv_out_height = (seq_len - kernel_size + 1) / stride; // 28
        conv_out_width = filters; // 8
        fc_in_size = conv_out_height * conv_out_width + ext_features; // 224 + 4 = 228

        net << convolutional_layer(seq_len, ts_features, kernel_size, 1, filters, padding::valid) // [30, 4] -> [28, 8]
            << relu_layer()
            << flatten_layer(conv_out_height, conv_out_width) // [28, 8] -> [224]
            << fully_connected_layer(fc_in_size, 1) // [224 + 4] -> [1]
            << sigmoid_layer();
    }

    void TrendReversalPredictor::train(const std::vector<Sample>& dataset, int epochs, float lr) {
        using namespace tiny_dnn;
        adam optimizer;
        optimizer.alpha = lr;
        const int batch_size = 8;

        std::random_device rd;
        std::mt19937 g(rd());

        for (int epoch = 0; epoch < epochs; ++epoch) {
            std::shuffle(dataset.begin(), dataset.end(), g);
            float total_loss = 0.0f;
            size_t batch_count = 0;
            for (size_t i = 0; i < dataset.size(); i += batch_size) {
                auto batch_end = std::min(i + batch_size, dataset.size());
                std::vector<vec_t> batch_data;
                std::vector<vec_t> batch_labels; // Changed to vec_t
                for (size_t j = i; j < batch_end; ++j) {
                    vec_t input = dataset[j].ts_data;
                    input.insert(input.end(), dataset[j].ext_data.begin(), dataset[j].ext_data.end());
                    batch_data.push_back(input);
                    batch_labels.push_back({ static_cast<float_t>(dataset[j].label) }); // Convert label_t to vec_t
                }
                bool success = net.train<bce_loss>(optimizer, batch_data, batch_labels, batch_size, 1);
                if (!success) {
                    std::cerr << "Training failed for batch " << batch_count + 1 << "\n";
                    continue;
                }
                float batch_loss = 0.0f;
                for (size_t j = 0; j < batch_data.size(); ++j) {
                    vec_t output = net.predict(batch_data[j]);
                    batch_loss += bce_loss::f(output, batch_labels[j]); // Use vec_t for target
                }
                batch_loss /= batch_data.size();
                total_loss += batch_loss;
                batch_count++;
            }
            std::cout << "Epoch " << epoch + 1 << ", Loss: " << total_loss / batch_count << "\n";
        }
        net.save("trend_model.bin");
    }

    void TrendReversalPredictor::update_context(const tiny_dnn::vec_t& new_ts_data, const tiny_dnn::vec_t& new_ext_data, float price_min, float max) {
        if (new_ts_data.size() == ts_features) {
            std::copy(ts_buffer.begin() + ts_features, ts_buffer.end(), ts_buffer.begin());
            vec_t normalized_ts = new_ts_data;
            normalize(normalized_ts, price_min, max);
            std::copy(normalized_ts.begin(), normalized_ts.end(), ts_buffer.end() - ts_features);
        }
        if (new_ext_data.size() == ext_features) {
            ext_buffer = new_ext_data;
        }
    }

    float TrendReversalPredictor::predict() {
        using namespace tiny_dnn;
        vec_t input = ts_buffer;
        input.insert(input.end(), ext_buffer.begin(), ext_buffer.end());

        auto start = std::chrono::high_resolution_clock::now();
        auto output = net.predict(input);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Inference time: " << duration << " ms\n";

        return output[0];
    }

    std::vector<Sample> TrendReversalPredictor::load_dataset(const std::string& file_path, size_t seq_len, size_t ts_features) {
        std::vector<Sample> dataset;
        std::ifstream file(file_path);
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            vec_t ts_data(seq_len * ts_features);
            vec_t ext_data(4);
            label_t label;
            for (size_t i = 0; i < seq_len * ts_features; ++i) {
                ss >> ts_data[i];
                if (ss.peek() == ',') ss.ignore();
            }
            for (size_t i = 0; i < 4; ++i) {
                ss >> ext_data[i];
                if (ss.peek() == ',') ss.ignore();
            }
            ss >> label;
            dataset.push_back({ ts_data, ext_data, label });
        }
        return dataset;
    }


}
