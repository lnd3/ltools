#pragma once

#include <vector>
#include <iostream>

#ifdef LDEPS_USE_LIBTORCH
#include <torch/torch.h>
#endif

namespace l::nn::libtorch {
    class Transformer {
    public:
        void process(const std::vector<float>& window);
    };

}

