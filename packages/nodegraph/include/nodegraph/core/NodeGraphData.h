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

    enum class OutputType {
        Default, // node will be processed if it is connected to the groups output by some route
        ExternalOutput, // node does not have meaningful output for other nodes but should still be processed (ex speaker output only has input)
        ExternalVisualOutput,
    };

    std::pair<float, float> GetInputBounds(InputBound bound);

    class NodeGraphBase;

    union Input {
        NodeGraphBase* mInputNode = nullptr;
        float* mInputFloat;
        float mInputFloatConstant;
    };

    /**********************************************************************************/
    class NodeDataIterator {
    public:
        NodeDataIterator(float* data = nullptr, float stepPerIndex = 1.0f) {
            mData = data;
            mIncrement = stepPerIndex;
            mIndex = mIncrement * 0.5f; // warmstart accumulator half a step to avoid rounding down (to previous index) close to whole integers because of floating point inprecision
        }
        ~NodeDataIterator() = default;

        float& operator*() {
            return *(mData + static_cast<int32_t>(mIndex));
        }
        float* operator++(int) {
            auto p = mData + static_cast<int32_t>(mIndex);
            mIndex += mIncrement;
            return p;
        }
        float& operator[](int offset) {
            return *(mData + static_cast<int32_t>(offset * offset));
        }
        float* data() {
            return mData;
        }
        void Reset(float* data, float stepPerIndex = 1.0f) {
            mData = data;
            mIncrement = stepPerIndex;
        }
    protected:
        int32_t mSize = 0;
        float mIndex = 0.0f;
        float* mData = nullptr;
        float mIncrement = 0.0f;
    };

    /*********************************************************************************/
    enum class InputTypeBase {
        SAMPLED = 0, // todo: make it interpolate in smaller custom buffers
        //SAMPLED_RWA = 0, // todo: add a smoothed sampled variant. Will replace interp_rwa and interp_rwa_ms as we will add a separate config for passing ticks or millis
        INTERP_RWA, // TBR
        INTERP_RWA_MS, // TBR
        CONSTANT_VALUE, // Will basically be replace by sampled as it should be able to handle 1-sized arrays
        CONSTANT_ARRAY, // Same here, will be replaced
        CUSTOM_INTERP_TWEEN, // custom input vars should not be used at all
        CUSTOM_INTERP_TWEEN_MS,
        CUSTOM_INTERP_RWA_MS
    };

    union InputUnion {
        l::audio::FilterRWAFloat mFilterRWA;
        l::math::tween::DynamicTween mTween;
        NodeDataIterator mIterator;

        InputUnion() : mFilterRWA() {}
        ~InputUnion() = default;
    };

}

