#pragma once

#include <tiny_dnn/tiny_dnn.h>

#include <vector>
#include <string>

namespace l::nn::tinydnn {

    struct Sample {
        tiny_dnn::vec_t ts_data; // OHLC: seq_len * 4
        tiny_dnn::vec_t ext_data; // Support/resistance: 4
        tiny_dnn::label_t label; // 0 or 1
    };

    class TrendReversalPredictor {
    private:
        size_t seq_len;
        size_t ts_features;
        size_t kernel_size, stride, conv_filters;
        size_t conv_out_height, conv_out_width;
        size_t fc_in_size;
        tiny_dnn::vec_t ts_buffer;
        tiny_dnn::network<tiny_dnn::sequential> net;

    public:
        TrendReversalPredictor(size_t seq_len_, size_t ts_features_);
        void train(std::vector<Sample>& dataset, int epochs = 50, float lr = 0.001);
        void update_context(const tiny_dnn::vec_t& new_ts_data, float price_min, float price_max);
        float predict();
        void normalize(tiny_dnn::vec_t& data, float min, float max);
        static std::vector<Sample> load_dataset(const std::string& file_path, size_t seq_len, size_t ts_features);
    };
}
