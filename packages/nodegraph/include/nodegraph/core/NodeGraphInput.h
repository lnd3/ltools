#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"
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
        INTERPOLATED,
        INTERPOLATED_MS,
        CONSTANT_VALUE,
        CONSTANT_ARRAY,
        CUSTOM_VALUE_INTERP_MS
    };

    union InputUnion {
        l::audio::FilterRWAFloat mFilter;
        NodeInputDataIterator mIterator;

        InputUnion() : mFilter() {}
        ~InputUnion() = default;
    };

    class NodeInput {
    public:
        NodeInput(InputTypeBase type, int32_t inputIndex) :
            mType(type),
            mInputIndex(inputIndex)
        {
            switch(mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                //new (&s.vec) std::vector<int>;
                new (&mInput.mFilter) l::audio::FilterRWA<float>();

                //mInput.mFilter = l::audio::FilterRWA<float>();
                break;
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                new (&mInput.mIterator) NodeInputDataIterator();
                //mInput.mIterator = NodeInputDataIterator();
                break;
            }
        }
        ~NodeInput() = default;

        void SetConvergence(float value) {
            switch(mType) {
            case InputTypeBase::INTERPOLATED:
                mInput.mFilter.SetConvergence(value);
                break;
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                mInput.mFilter.SetConvergenceInMs(value);
                break;
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                ASSERT(false) << "Failed to set convergence on a non interpolated input type";
                break;
            }
        }

        void SetTarget(float value) {
            switch (mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                mInput.mFilter.SetTarget(value);
                break;
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                break;
            }
        }

        void SetValue(float value) {
            switch (mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                mInput.mFilter.Value() = value;
                break;
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                break;
            }
        }

        float GetValueNext() {
            switch (mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                return mInput.mFilter.Next();
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_ARRAY:
                return *mInput.mIterator++;
            case InputTypeBase::CONSTANT_VALUE:
                return *mInput.mIterator;
            }
            return 0.0f;
        }

        float GetValue() {
            switch (mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                return mInput.mFilter.Value();
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                return *mInput.mIterator;
            }
            return 0.0f;
        }

        float GetArrayValue(int32_t index) {
            if (mType == InputTypeBase::CONSTANT_ARRAY) {
                return mInput.mIterator[index];
            }
            return 0.0f;
        }

        float* GetArray() {
            if (mType == InputTypeBase::CONSTANT_ARRAY) {
                return mInput.mIterator.data();
            }
            return nullptr;
        }

        // run on each new batch call to setup input iterators for buffered data
        void ProcessUpdate(std::vector<NodeGraphInput>& input, int32_t numSamples) {
            switch (mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS:
                break;
            case InputTypeBase::SAMPLED:
                mInput.mIterator = input.at(mInputIndex).GetBufferIterator(numSamples);
                break;
            case InputTypeBase::CONSTANT_VALUE:
                mInput.mIterator.Reset(&input.at(mInputIndex).Get(), 1);
                break;
            case InputTypeBase::CONSTANT_ARRAY:
                mInput.mIterator = input.at(mInputIndex).GetArrayIterator();
                break;
            }
        }

        // run on each node update (can be node specific) and will update node rwa filters
        void NodeUpdate(std::vector<NodeGraphInput>& input) {
            switch (mType) {
            case InputTypeBase::INTERPOLATED:
            case InputTypeBase::INTERPOLATED_MS:
                mInput.mFilter.SetTarget(input.at(mInputIndex).Get());
                break;
            case InputTypeBase::CUSTOM_VALUE_INTERP_MS: // must set it manually
            case InputTypeBase::SAMPLED:
            case InputTypeBase::CONSTANT_VALUE:
            case InputTypeBase::CONSTANT_ARRAY:
                break;
            }
        }

    protected:
        InputTypeBase mType;
        int32_t mInputIndex = 0;
        InputUnion mInput;
    };

}

