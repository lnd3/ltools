#include "testing/Test.h"
#include "logging/Log.h"

#ifdef HAS_LIBTORCH
#include <nn/torch/TorchBase.h>

#include <random>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

TEST(Torch1, Basic) {
    return 0;
    l::nn::libtorch::Transformer tf;
    std::vector<float> input;

    std::random_device rdev;

    srand(45924296592);
    float acc = 1.0f;
    for (int32_t i = 0; i < 100; i++) {
        input.push_back(acc);
        acc *= 1.0f + 0.1f * (rand() / static_cast<float>(RAND_MAX) - 0.5f);
    }
    std::vector<float> output;
    tf.process(input);

    return 0;
}
#endif

