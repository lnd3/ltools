#include "testing/Test.h"
#include "logging/Log.h"

#include <nn/fann/NNFannBase.h>
#include <nn/SlidingWindowBuffer.h>

using namespace l::nn;
using namespace l::nn::fann;


TEST(Fann, SlidingWindow) {

    // Setup sliding window for 10 seconds at 1 sample per second
    SlidingWindowBuffer sliding_window(10, 1);

    // Create a node that expects input size = 4 (10 seconds * 1 sample/sec * 4 features)
    // Actually, since sliding_window concatenates all inputs in window, if each input is 1 feature:
    // input_size = window_size * sample_rate * features_per_sample
    const unsigned features_per_sample = 1;
    const unsigned input_size = 10 * 1 * features_per_sample;
    const unsigned output_size = 1;

     NNFannBase node("example_node", input_size, output_size);

    // Feed data into the sliding window over time
    for (UnixTime t = 1000; t < 1020; ++t) {
        InputVec sample = { static_cast<float>(t % 10) };  // dummy input feature
        sliding_window.push(t, sample);

        // Once we have a full window, prepare training example
        auto window_input = sliding_window.getWindow();
        if (window_input) {
            TrainingExample ex;
            ex.time = sliding_window.currentTimeRange();
            ex.input = *window_input;
                // Dummy target for demo
            ex.target = {0.5f};

            if (node.trainIfNeeded(ex)) {
                std::cout << "Trained on time range [" << ex.time.start << ", " << ex.time.end << ")\n";
            }
            else {
                std::cout << "Already trained on this time range\n";
            }

            // Try inference
            auto pred = node.infer(ex.input);
            if (pred) {
                std::cout << "Inference output: " << (*pred)[0] << "\n";
            }
        }
    }

    return 0;
}


