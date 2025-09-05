#pragma once
#include <vector>
#include <string>
#include <tiny_dnn/tiny_dnn.h>

namespace l::nn::tinydnn {

    struct Sample {
        tiny_dnn::vec_t ts_data; // OHLC: seq_len * 4
        tiny_dnn::vec_t ext_data; // Support/resistance: 4
        tiny_dnn::label_t label; // 0 or 1
    };

    class TrendReversalPredictor {
    private:
        tiny_dnn::network<tiny_dnn::sequential> net;
        tiny_dnn::vec_t ts_buffer; // OHLC sliding window
        tiny_dnn::vec_t ext_buffer; // Support/resistance
        size_t seq_len, ts_features, ext_features, conv_out_height, conv_out_width, fc_in_size;

    public:
        TrendReversalPredictor(size_t seq_len, size_t ts_features, size_t ext_features);
        void train(const std::vector<Sample>& dataset, int epochs = 50, float lr = 0.001);
        void update_context(const tiny_dnn::vec_t& new_ts_data, const tiny_dnn::vec_t& new_ext_data, float price_min, float price_max);
        float predict();
        void normalize(tiny_dnn::vec_t& data, float min, float max);
        static std::vector<Sample> load_dataset(const std::string& file_path, size_t seq_len, size_t ts_features);
    };
}
