#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

namespace l::nodegraph {

    enum class InputType {
        INPUT_EMPTY,
        INPUT_NODE,
        INPUT_CONSTANT,
        INPUT_VALUE,
        INPUT_ARRAY
    };

    enum class InputBound {
        INPUT_DONTCHANGE = 0,
        INPUT_UNBOUNDED,
        INPUT_0_TO_1,
        INPUT_0_TO_2,
        INPUT_NEG_1_POS_1,
        INPUT_0_100,
        INPUT_CUSTOM,
    };

    std::pair<float, float> GetInputBounds(InputBound bound);

    class NodeGraphBase;

    union Input {
        NodeGraphBase* mInputNode = nullptr;
        float* mInputFloat;
        float mInputFloatConstant;
        int32_t* mInputInt;
        int32_t mInputIntConstant;
    };

    struct NodeGraphInput {
        Input mInput;
        InputType mInputType = InputType::INPUT_EMPTY;

        float mBoundMin = -l::math::constants::FLTMAX;
        float mBoundMax = l::math::constants::FLTMAX;

        int8_t mInputFromOutputChannel = 0;
        std::unique_ptr<std::string> mName;

        // hack to get input buffers working
        std::unique_ptr<std::vector<float>> mInputBuf = nullptr;

        void Reset();
        bool HasInputNode();
        float Get();
        float& Get(int32_t numSamples);
        int32_t GetSize();
    };

}

