#include <nn/tinydnn/TrendReversalPredictor.h>

#include <nn/tinydnn/flatten_layer.h>

#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

namespace l::nn::tinydnn {


    // Binary Cross Entropy Loss
    class bce_loss {
    public:
        static float_t f(const vec_t& y, const vec_t& t) {
            if (y.size() != 1 || t.size() != 1) throw std::runtime_error("BCE requires scalar output.");
            float_t y_val = y[0], t_val = t[0];
            return -t_val * log(y_val + 1e-10f) - (1.0f - t_val) * log(1.0f - y_val + 1e-10f);
        }

        static vec_t df(const vec_t& y, const vec_t& t) {
            if (y.size() != 1 || t.size() != 1) throw std::runtime_error("BCE requires scalar output.");
            float_t y_val = y[0], t_val = t[0];
            float_t grad = (y_val - t_val) / ((y_val + 1e-10f) * (1.0f - y_val + 1e-10f));
            return { grad };
        }
    };

    TrendReversalPredictor::TrendReversalPredictor(size_t seq_len_, size_t ts_features_)
        : seq_len(seq_len_), ts_features(ts_features_),
        ts_buffer(seq_len* ts_features, 0.0f) {
        using namespace tiny_dnn;
        // Layer config parameters
        kernel_size = 3;
        stride = 1;
        conv_filters = 8;

        conv_out_height = (seq_len - kernel_size) / stride + 1;  // e.g. 28
        conv_out_width = conv_filters;  // e.g. 8

        fc_in_size = conv_out_height * conv_out_width * 2;  // flatten output size, no ext_features

        net << convolutional_layer(seq_len, ts_features, kernel_size, 1, conv_filters, padding::valid)
            << relu_layer()
            << flatten_layer(conv_out_height, 2, conv_out_width)
            << fully_connected_layer(fc_in_size, 1)
            << sigmoid_layer();
    }

    void TrendReversalPredictor::normalize(vec_t& data, float min, float max) {
        if (max == min) return;
        for (auto& val : data) val = (val - min) / (max - min);
    }

    void TrendReversalPredictor::train(std::vector<Sample>& dataset, int epochs, float lr) {
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
                size_t batch_end = std::min(i + batch_size, dataset.size());
                std::vector<vec_t> batch_data;
                std::vector<vec_t> batch_labels;

                for (size_t j = i; j < batch_end; ++j) {
                    vec_t input = dataset[j].ts_data;
                    input.insert(input.end(), dataset[j].ext_data.begin(), dataset[j].ext_data.end());
                    batch_data.push_back(input);
                    batch_labels.push_back({ static_cast<float_t>(dataset[j].label) });
                }

                bool success = net.train<bce_loss>(optimizer, batch_data, batch_labels, batch_size, 1);
                if (!success) {
                    std::cerr << "Training failed for batch " << batch_count + 1 << "\n";
                    continue;
                }

                float batch_loss = 0.0f;
                for (size_t j = 0; j < batch_data.size(); ++j) {
                    vec_t output = net.predict(batch_data[j]);
                    batch_loss += bce_loss::f(output, batch_labels[j]);
                }
                batch_loss /= batch_data.size();
                total_loss += batch_loss;
                ++batch_count;
            }

            std::cout << "Epoch " << epoch + 1 << ", Loss: " << total_loss / batch_count << "\n";
        }

        net.save("trend_model.bin");
    }

    void TrendReversalPredictor::update_context(const vec_t& new_ts_data, float price_min, float price_max) {
        if (new_ts_data.size() == ts_features) {
            std::copy(ts_buffer.begin() + ts_features, ts_buffer.end(), ts_buffer.begin());
            auto normalized_ts = new_ts_data;
            normalize(normalized_ts, price_min, price_max);
            std::copy(normalized_ts.begin(), normalized_ts.end(), ts_buffer.end() - ts_features);
        }
    }

    float TrendReversalPredictor::predict() {
        using namespace tiny_dnn;
        return net.predict(ts_buffer)[0];
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
