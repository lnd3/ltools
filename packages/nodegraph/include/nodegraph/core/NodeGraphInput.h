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

    class NodeInputDataIterator {
    public:
        NodeInputDataIterator(float* data, int32_t size) {
            mData = data;
            mSize = size;
            mIncrement = size > 1 ? 1 : 0;
        }
        float& operator*() {
            return *mData;
        }
        float* operator++(int) {
            float* data = mData;
            mData += mIncrement;
            return data;
        }

    protected:
        float* mData = nullptr;
        int32_t mSize = 0;
        int32_t mIncrement = 0;
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
        float& Get(int32_t numSamples = 1);
        NodeInputDataIterator GetIterator(int32_t numSamples = 1);
        int32_t GetSize();
    };

}

