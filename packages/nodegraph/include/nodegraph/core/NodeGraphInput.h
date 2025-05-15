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
        NodeGraphInput() {

        }

        ~NodeGraphInput() noexcept {
            switch (mInputType) {
            case InputType::INPUT_NODE:
                mInput.mInputNode = nullptr;
                break;
            case InputType::INPUT_ARRAY:
                if (mInput.mInputFloatBuf) {
                    delete mInput.mInputFloatBuf;
                }
                break;
            case InputType::INPUT_TEXT:
                if (mInput.mInputTextBuf) {
                    delete mInput.mInputTextBuf;
                }
                break;
            case InputType::INPUT_CONSTANT:
            case InputType::INPUT_VALUE:
            default:
                break;
            }
        }

        void Clear();
        void Reset();
        bool IsOfType(InputType type);

        bool HasInputNode();
        bool HasInputNode(NodeGraphBase* node);
        bool HasInputValue(float* floatPtr);

        float& Get(int32_t minSize = 1, int32_t offset = 0);
        float& GetArray(int32_t minSize = 1, int32_t offset = 0);
        std::string_view GetText(int32_t minSize = 16);
        NodeGraphBase* GetInputNode();

        NodeDataIterator GetIterator(int32_t minSize = 1);
        int32_t GetSize();

        void SetConstant(float constant);
        void SetValue(float* floatPtr);
        void SetArray(float defaultValue, int32_t minSize);
        void SetText(std::string_view text);
        bool SetInputNode(NodeGraphBase* source, int8_t sourceOutputChannel);

        float mBoundMin = -l::math::constants::FLTMAX;
        float mBoundMax = l::math::constants::FLTMAX;
        float mInputLod = 1.0f; // buffer size level of detail  value[1.0f, buffer size] (if 1 it will write all generated values to the buffer, if 'buffer size' it will only have the latest written value),
        int8_t mInputFromOutputChannel = 0;
    protected:
        Input mInput;
        InputType mInputType = InputType::INPUT_CONSTANT;
    };


    /*********************************************************************************/

    class InputAccessor {
    public:
        InputAccessor(InputIterationType type, int32_t inputIndex) :
            mType(type),
            mInputIndex(inputIndex)
        {
        }
        ~InputAccessor() = default;

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
        InputIterationType mType;
        int32_t mInputIndex = 0;
        InputUnion mInput;
    };

}

