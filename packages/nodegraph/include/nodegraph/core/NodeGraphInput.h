#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathAll.h"
#include "audio/AudioUtils.h"

#include "nodegraph/core/NodeGraphData.h"

namespace l::nodegraph {

    class NodeGraphInput {
    public:
        Input mInput;
        InputType mInputType = InputType::INPUT_CONSTANT;

        float mBoundMin = -l::math::constants::FLTMAX;
        float mBoundMax = l::math::constants::FLTMAX;

        int8_t mInputFromOutputChannel = 0;
        std::unique_ptr<std::string> mName = nullptr;

        // hack to get input buffers working
        std::unique_ptr<std::vector<float>> mInputBuf = nullptr;
        float mInputLod = 1.0f; // buffer size level of detail  value[1.0f, buffer size] (if 1 it will write all generated values to the buffer, if 'buffer size' it will only have the latest written value),

        void Reset();
        bool HasInputNode();
        float& Get(int32_t size = 1);
        NodeDataIterator GetIterator(int32_t size = 1);
        NodeDataIterator GetArrayIterator();
        int32_t GetSize();
    };


    /*********************************************************************************/

    class NodeGraphInputAccessor {
    public:
        NodeGraphInputAccessor(InputTypeBase type, int32_t inputIndex) :
            mType(type),
            mInputIndex(inputIndex)
        {
        }
        ~NodeGraphInputAccessor() = default;

        void SetUpdateRate(float updateRate);
        void SetDuration(float ms, float limit = 0.001f);
        void SetDuration(int32_t ticks);
        void SetTarget(float value);
        void SetValue(float value);
        float GetValueNext();
        float GetValue();
        float GetArrayValue(int32_t index);
        float* GetArray();
        void BatchUpdate(std::vector<NodeGraphInput>& input, int32_t numSamples);
        void NodeUpdate(std::vector<NodeGraphInput>& input, float updateRate);

    protected:
        InputTypeBase mType;
        int32_t mInputIndex = 0;
        InputUnion mInput;
    };

}

