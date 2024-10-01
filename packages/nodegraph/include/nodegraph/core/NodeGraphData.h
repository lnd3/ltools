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

    enum class NodeType {
        Default, // node will be processed if it is connected to the groups output by some route
        ExternalOutput, // node does not have meaningful output for other nodes but should still be processed (ex speaker output only has input)
        ExternalVisualOutput, // node has visual output that requires special handling, for example the graph plot node
        ExternalInput
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
        NodeDataIterator(float* data = nullptr, float increment = 1.0f) {
            mData = data;
            mIncrement = increment;
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
            return *(mData + static_cast<int32_t>(offset * mIncrement));
        }
        float* data() {
            return mData;
        }
        void Reset(float* data, float stepPerIndex = 1.0f) {
            mData = data;
            mIncrement = stepPerIndex;
        }

        float GetStepsPerIncrement() {
            if (mIncrement == 0.0f) {
                return 0.0f;
            }
            return 1.0f / mIncrement;
        }
    protected:
        float* mData = nullptr;
        float mIndex = 0.0f;
        float mIncrement = 0.0f;
    };

    class NodeDataIteratorRwa {
    public:
        NodeDataIteratorRwa(NodeDataIterator&& iterator) {
            Reset(std::move(iterator));
        }
        float operator*() {
            return mRwa.Value();
        }
        float operator++(int) {
            mRwa.SetTarget(*mIterator++);
            return mRwa.Next();
        }
        void Reset(NodeDataIterator&& iterator) {
            mIterator = std::move(iterator);
            mRwa.Value() = *mIterator;
            mRwa.SetConvergenceInTicks(l::math::functions::max2(4.0f, iterator.GetStepsPerIncrement()), 0.35f);
        }
    protected:
        NodeDataIterator mIterator;
        l::audio::FilterRWAFloat mRwa;
    };
    /*********************************************************************************/
    enum class InputIterationType {
        SAMPLED = 0, // interpolate in a buffer the size of a ProcessSubGraph(size) call. The actual size is defined by the lod factor of the source output buffer.
        SAMPLED_RWA, // same as SAMPLED, but it also uses RWA on the output with a smoothing factor defined by the lod factor of the source output buffer
        CONSTANT_ARRAY, // user defined array for custom usage
        CUSTOM_INTERP_TWEEN, // custom input vars that tweens the input like a s curve
        CUSTOM_INTERP_TWEEN_MS, // millisecond synchronized tweening
        CUSTOM_INTERP_RWA_MS // custom input var for rwa smoothing synchronized to milliseconds
    };

    union InputUnion {
        l::audio::FilterRWAFloat mFilterRWA;
        l::math::tween::DynamicTween mTween;
        NodeDataIterator mIterator;
        NodeDataIteratorRwa mIteratorRwa;

        InputUnion() : mFilterRWA() {}
        ~InputUnion() = default;
    };

}

