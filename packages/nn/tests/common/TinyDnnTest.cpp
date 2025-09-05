#include "testing/Test.h"
#include "logging/Log.h"

#include <nn/tinydnn/TrendReversalPredictor.h>

#include <iostream>

using namespace l::nn::tinydnn;

TEST(TinyDnn, Basic) {

    size_t seq_len = 30;
    size_t ts_features = 4;
    size_t ext_features = 4;

    TrendReversalPredictor predictor(seq_len, ts_features, ext_features);
    auto dataset = TrendReversalPredictor::load_dataset("dataset/data.csv", seq_len, ts_features);

    float price_min = 90.0f, price_max = 110.0f;
    for (auto& sample : dataset) {
        predictor.normalize(sample.ts_data, price_min, price_max);
    }

    predictor.train(dataset, 50, 0.001);

    tiny_dnn::vec_t new_ts_data = { 100.5f, 101.0f, 100.0f, 100.8f };
    tiny_dnn::vec_t new_ext_data = { 95.0f, 105.0f, 5.8f, 4.2f };
    predictor.update_context(new_ts_data, new_ext_data, price_min, price_max);
    float prob = predictor.predict();
    std::cout << "Trend reversal probability: " << prob * 100 << "%\n";

    return 0;

}


