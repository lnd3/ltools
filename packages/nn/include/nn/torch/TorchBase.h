#pragma once

#include <vector>
#include <iostream>

#ifdef HAS_LIBTORCH
#include <torch/all.h>
#endif

namespace l::nn::libtorch {
    class Transformer {
    public:
        void process(const std::vector<float>& window);
    };

}

