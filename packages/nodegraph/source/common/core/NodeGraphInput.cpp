#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    void NodeGraphInput::Clear() {
        if (mInputBuf) {
            mInputBuf.reset();
        }
        mInput.mInputNode = nullptr;
        mInputType = InputType::INPUT_CONSTANT;
        mInputFromOutputChannel = 0;
    }

    void NodeGraphInput::Reset() {
        if (mInputType == InputType::INPUT_NODE || mInputType == InputType::INPUT_VALUE) {
            mInput.mInputNode = nullptr;
            mInputType = InputType::INPUT_CONSTANT;
            mInputFromOutputChannel = 0;
        }
    }

    bool NodeGraphInput::HasInputNode() {
        if (mInputType == InputType::INPUT_NODE) {
            return true;
        }
        return false;
    }

    float& NodeGraphInput::Get(int32_t size) {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutput(mInputFromOutputChannel, size);
            }
            break;
        case InputType::INPUT_ARRAY:
            if (!mInputBuf) {
                mInputBuf = std::make_unique<std::vector<float>>();
            }
            if (static_cast<int32_t>(mInputBuf->size()) < size) {
                mInputBuf->resize(size);
                for (size_t i = 0; i < mInputBuf->size();i++) {
                    (*mInputBuf)[i] = 0.0f;
                }
            }
            return *mInputBuf->data();
        case InputType::INPUT_CONSTANT:
            return mInput.mInputFloatConstant;
        case InputType::INPUT_VALUE:
            return *mInput.mInputFloat;
        }
        return mInput.mInputFloatConstant;
    }

    NodeDataIterator NodeGraphInput::GetIterator(int32_t size) {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutputOf(mInputFromOutputChannel).GetIterator();
            }
            break;
        case InputType::INPUT_ARRAY:
            if (!mInputBuf) {
                mInputBuf = std::make_unique<std::vector<float>>();
            }
            if (static_cast<int32_t>(mInputBuf->size()) < size) {
                mInputBuf->resize(size);
                for (size_t i = 0; i < mInputBuf->size(); i++) {
                    (*mInputBuf)[i] = 0.0f;
                }
            }
            return NodeDataIterator(mInputBuf->data());
        case InputType::INPUT_VALUE:
            return NodeDataIterator(mInput.mInputFloat, 0.0f);
        case InputType::INPUT_CONSTANT:
            return NodeDataIterator(&mInput.mInputFloatConstant, 0.0f);
        }
        return NodeDataIterator(&mInput.mInputFloatConstant, 0.0f);
    }

    NodeDataIterator NodeGraphInput::GetArrayIterator() {
        auto size = GetSize();
        return NodeDataIterator(&Get(size));
    }

    int32_t NodeGraphInput::GetSize() {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutputSize(mInputFromOutputChannel);
            }
            break;
        case InputType::INPUT_ARRAY:
            if (mInputBuf) {
                return static_cast<int32_t>(mInputBuf->size());
            }
            return 1;
        case InputType::INPUT_CONSTANT:
            return 1;
        case InputType::INPUT_VALUE:
            return 1;
        }
        return 1;
    }

    /*********************************************************************************/

    void InputAccessor::SetUpdateRate(float updateRate) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.Update(updateRate);
            break;
        case InputIterationType::CUSTOM_INTERP_RWA_MS: // must set it manually
            mInput.mFilterRWA.SetRWAUpdateRate(updateRate);
            break;
        case InputIterationType::SAMPLED:
        case InputIterationType::CONSTANT_ARRAY:
        case InputIterationType::SAMPLED_RWA:
            break;
        }
    }

    void InputAccessor::SetDuration(int32_t ticks) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.SetConvergenceInTicks(static_cast<float>(ticks));
            break;
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.SetTweenLength(ticks);
            break;
        case InputIterationType::SAMPLED:
        case InputIterationType::SAMPLED_RWA:
        case InputIterationType::CONSTANT_ARRAY:
            break;
        }
    }

    void InputAccessor::SetDuration(float ms, float limit) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.SetConvergenceInMs(ms, limit);
            break;
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.SetTweenLength(l::audio::GetAudioTicksFromMS(ms));
            break;
        case InputIterationType::SAMPLED:
        case InputIterationType::SAMPLED_RWA:
        case InputIterationType::CONSTANT_ARRAY:
            break;
        }
    }

    void InputAccessor::SetTarget(float value) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.SetTarget(value);
            break;
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.SetTarget(value);
            break;
        case InputIterationType::SAMPLED:
        case InputIterationType::SAMPLED_RWA:
        case InputIterationType::CONSTANT_ARRAY:
            break;
        }
    }

    void InputAccessor::SetValue(float value) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.Value() = value;
            break;
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.Reset(value);
            break;
        case InputIterationType::SAMPLED:
        case InputIterationType::SAMPLED_RWA:
        case InputIterationType::CONSTANT_ARRAY:
            break;
        }
    }

    float InputAccessor::GetValueNext() {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
            return mInput.mFilterRWA.Next();
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            return mInput.mTween.Next();
        case InputIterationType::SAMPLED:
        case InputIterationType::CONSTANT_ARRAY:
            return *mInput.mIterator++;
        case InputIterationType::SAMPLED_RWA:
            return mInput.mIteratorRwa++;
        }
        return 0.0f;
    }

    float InputAccessor::GetValue() {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
            return mInput.mFilterRWA.Value();
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            return mInput.mTween.Value();
        case InputIterationType::SAMPLED:
        case InputIterationType::CONSTANT_ARRAY:
            return *mInput.mIterator;
        case InputIterationType::SAMPLED_RWA:
            return *mInput.mIteratorRwa;
        }
        return 0.0f;
    }

    float InputAccessor::GetArrayValue(int32_t index) {
        return mInput.mIterator[index];
    }

    float* InputAccessor::GetArray() {
        return mInput.mIterator.data();
    }

    // run on each new batch call to setup input iterators for buffered data
    void InputAccessor::BatchUpdate(std::vector<NodeGraphInput>& input, int32_t numSamples) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_RWA_MS:
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            break;
        case InputIterationType::SAMPLED:
            mInput.mIterator = input.at(mInputIndex).GetIterator(numSamples);
            break;
        case InputIterationType::SAMPLED_RWA:
            mInput.mIteratorRwa = NodeDataIteratorRwa(input.at(mInputIndex).GetIterator(numSamples));
            break;
        case InputIterationType::CONSTANT_ARRAY:
            mInput.mIterator = input.at(mInputIndex).GetArrayIterator();
            break;
        }
    }

    // run on each node update (can be node specific) and will update node rwa filters
    void InputAccessor::NodeUpdate(std::vector<NodeGraphInput>&, float updateRate) {
        switch (mType) {
        case InputIterationType::CUSTOM_INTERP_TWEEN:
        case InputIterationType::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.Update(updateRate);
            break;
        case InputIterationType::CUSTOM_INTERP_RWA_MS: // must set it manually
        case InputIterationType::SAMPLED:
        case InputIterationType::SAMPLED_RWA:
        case InputIterationType::CONSTANT_ARRAY:
            break;
        }
    }
}