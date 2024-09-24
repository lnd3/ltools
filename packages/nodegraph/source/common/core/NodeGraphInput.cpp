#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

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
            if (!mInputBuf) {
                return 1;
            }
            return static_cast<int32_t>(mInputBuf->size());
        case InputType::INPUT_CONSTANT:
            return 1;
        case InputType::INPUT_VALUE:
            return 1;
        }
        return 1;
    }

    /*********************************************************************************/

    void NodeGraphInputAccessor::SetUpdateRate(float updateRate) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
            mInput.mFilterRWA.SetRWAUpdateRate(updateRate);
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.Update(updateRate);
            break;
        case InputTypeBase::CUSTOM_INTERP_RWA_MS: // must set it manually
            mInput.mFilterRWA.SetRWAUpdateRate(updateRate);
            break;
        case InputTypeBase::SAMPLED:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
        case InputTypeBase::SAMPLED_RWA:
            break;
        }
    }

    void NodeGraphInputAccessor::SetDuration(int32_t ticks) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
            mInput.mFilterRWA.SetConvergenceInTicks(static_cast<float>(ticks));
            break;
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.SetConvergenceInTicks(static_cast<float>(ticks));
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.SetTweenLength(ticks);
            break;
        case InputTypeBase::SAMPLED:
        case InputTypeBase::SAMPLED_RWA:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
            ASSERT(false) << "Failed to set convergence on a non interpolated input type";
            break;
        }
    }

    void NodeGraphInputAccessor::SetDuration(float ms, float limit) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
            mInput.mFilterRWA.SetConvergenceInMs(ms, limit);
            break;
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.SetConvergenceInMs(ms, limit);
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.SetTweenLength(l::audio::GetAudioTicksFromMS(ms));
            break;
        case InputTypeBase::SAMPLED:
        case InputTypeBase::SAMPLED_RWA:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
            ASSERT(false) << "Failed to set convergence on a non interpolated input type";
            break;
        }
    }

    void NodeGraphInputAccessor::SetTarget(float value) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.SetTarget(value);
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.SetTarget(value);
            break;
        case InputTypeBase::SAMPLED:
        case InputTypeBase::SAMPLED_RWA:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
            break;
        }
    }

    void NodeGraphInputAccessor::SetValue(float value) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            mInput.mFilterRWA.Value() = value;
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.Reset(value);
            break;
        case InputTypeBase::SAMPLED:
        case InputTypeBase::SAMPLED_RWA:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
            break;
        }
    }

    float NodeGraphInputAccessor::GetValueNext() {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            return mInput.mFilterRWA.Next();
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            return mInput.mTween.Next();
        case InputTypeBase::SAMPLED:
        case InputTypeBase::CONSTANT_ARRAY:
            return *mInput.mIterator++;
        case InputTypeBase::SAMPLED_RWA:
            return mInput.mIteratorRwa++;
        case InputTypeBase::CONSTANT_VALUE:
            return *mInput.mIterator;
        }
        return 0.0f;
    }

    float NodeGraphInputAccessor::GetValue() {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            return mInput.mFilterRWA.Value();
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            return mInput.mTween.Value();
        case InputTypeBase::SAMPLED:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
            return *mInput.mIterator;
        case InputTypeBase::SAMPLED_RWA:
            return *mInput.mIteratorRwa;
        }
        return 0.0f;
    }

    float NodeGraphInputAccessor::GetArrayValue(int32_t index) {
        if (mType == InputTypeBase::CONSTANT_ARRAY) {
            return mInput.mIterator[index];
        }
        return 0.0f;
    }

    float* NodeGraphInputAccessor::GetArray() {
        if (mType == InputTypeBase::CONSTANT_ARRAY) {
            return mInput.mIterator.data();
        }
        return nullptr;
    }

    // run on each new batch call to setup input iterators for buffered data
    void NodeGraphInputAccessor::BatchUpdate(std::vector<NodeGraphInput>& input, int32_t numSamples) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
        case InputTypeBase::CUSTOM_INTERP_RWA_MS:
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            break;
        case InputTypeBase::SAMPLED:
            mInput.mIterator = input.at(mInputIndex).GetIterator(numSamples);
            break;
        case InputTypeBase::SAMPLED_RWA:
            mInput.mIteratorRwa = input.at(mInputIndex).GetIterator(numSamples);
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
    void NodeGraphInputAccessor::NodeUpdate(std::vector<NodeGraphInput>& input, float updateRate) {
        switch (mType) {
        case InputTypeBase::INTERP_RWA:
        case InputTypeBase::INTERP_RWA_MS:
            mInput.mFilterRWA.SetTarget(input.at(mInputIndex).Get());
            break;
        case InputTypeBase::CUSTOM_INTERP_TWEEN:
        case InputTypeBase::CUSTOM_INTERP_TWEEN_MS:
            mInput.mTween.Update(updateRate);
            break;
        case InputTypeBase::CUSTOM_INTERP_RWA_MS: // must set it manually
            break;
        case InputTypeBase::SAMPLED:
        case InputTypeBase::SAMPLED_RWA:
        case InputTypeBase::CONSTANT_VALUE:
        case InputTypeBase::CONSTANT_ARRAY:
            break;
        }
    }
}