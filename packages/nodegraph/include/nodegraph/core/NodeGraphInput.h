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
    };

    /*********************************************************************************/

    class NodeInputDataIterator {
    public:
        NodeInputDataIterator(float* data = nullptr, int32_t size = 0) {
            mData = data;
            mSize = size;
            mIncrement = size > 1 ? 1 : 0;
        }
        ~NodeInputDataIterator() = default;

        float& operator*() {
            return *mData;
        }
        float* operator++(int) {
            float* data = mData;
            mData += mIncrement;
            return data;
        }
        float operator[](int index) {
            return mData[index];
        }
        float* data() {
            return mData;
        }
        void Reset(float* data, int32_t size) {
            mData = data;
            mSize = size;
            mIncrement = size > 1 ? 1 : 0;
        }

    protected:
        float* mData = nullptr;
        int32_t mSize = 0;
        int32_t mIncrement = 0;
    };

    /*********************************************************************************/

    class NodeGraphInput {
    public:
        Input mInput;
        InputType mInputType = InputType::INPUT_EMPTY;

        float mBoundMin = -l::math::constants::FLTMAX;
        float mBoundMax = l::math::constants::FLTMAX;

        int8_t mInputFromOutputChannel = 0;
        std::unique_ptr<std::string> mName = nullptr;

        // hack to get input buffers working
        std::unique_ptr<std::vector<float>> mInputBuf = nullptr;

        void Reset();
        bool HasInputNode();
        float& Get(int32_t numSamples = 1);
        NodeInputDataIterator GetBufferIterator(int32_t numSamples = 1);
        NodeInputDataIterator GetArrayIterator();
        int32_t GetSize();
    };


    /****************************************************************************************/
    enum class InputTypeBase {
        SAMPLED = 0,
        INTERP_RWA,
        INTERP_RWA_MS,
        INTERP_TWEEN,
        INTERP_TWEEN_MS,
        CONSTANT_VALUE,
        CONSTANT_ARRAY,
        CUSTOM_VALUE_INTERP_RWA_MS
    };

    union InputUnion {
        l::audio::FilterRWAFloat mFilterRWA;
        l::math::tween::DynamicTween mTween;
        NodeInputDataIterator mIterator;

        InputUnion() : mFilterRWA() {}
        ~InputUnion() = default;
    };

    /*********************************************************************************/

    class NodeGraphInputAccessor {
    public:
        NodeGraphInputAccessor(InputTypeBase type, int32_t inputIndex) :
            mType(type),
            mInputIndex(inputIndex)
        {
            switch(mType) {
            case InputTypeBase::INTERP_RWA:
            case InputTypeBase::INTERP_RWA_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_RWA_MS:
                //new (&s.vec) std::vector<int>;
                new (&mInput.mFilterRWA) l::audio::FilterRWA<float>();

                //mInput.mFilter = l::audio::FilterRWA<float>();
                break;
            case InputTypeBase::INTERP_TWEEN:
            case InputTypeBase::INTERP_TWEEN_MS:
                new (&mInput.mTween) l::math::tween::DynamicTween();
                break;
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                new (&mInput.mIterator) NodeInputDataIterator();
                //mInput.mIterator = NodeInputDataIterator();
                break;
            }
        }
        ~NodeGraphInputAccessor() = default;

        void SetDuration(float ms);
        void SetDuration(int32_t ticks);
        void SetTarget(float value);
        void SetValue(float value);
        float GetValueNext();
        float GetValue();
        float GetArrayValue(int32_t index);
        float* GetArray();
        void ProcessUpdate(std::vector<NodeGraphInput>& input, int32_t numSamples, float updateRate);
        void NodeUpdate(std::vector<NodeGraphInput>& input, float updateRate);

    protected:
        InputTypeBase mType;
        int32_t mInputIndex = 0;
        InputUnion mInput;
    };

}

