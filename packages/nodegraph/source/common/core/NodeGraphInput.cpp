#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    void NodeGraphInput::Reset() {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            mInputFromOutputChannel = 0;
            mInput.mInputNode = nullptr;
            break;
        case InputType::INPUT_ARRAY:
            if (mInput.mInputFloatBuf) {
                delete mInput.mInputFloatBuf;
                mInput.mInputFloatBuf = nullptr;
            }
            break;
        case InputType::INPUT_TEXT:
            if (mInput.mInputTextBuf) {
                delete mInput.mInputTextBuf;
                mInput.mInputTextBuf = nullptr;
            }
            break;
        case InputType::INPUT_CONSTANT:
            mInput.mInputFloatConstant = 0.0f;
            break;
        case InputType::INPUT_VALUE:
            break;
        }
        mInputType = InputType::INPUT_CONSTANT;
    }

    void NodeGraphInput::Clear() {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            mInputFromOutputChannel = 0;
            mInput.mInputNode = nullptr;
            break;
        case InputType::INPUT_ARRAY:
            if (mInput.mInputFloatBuf) {
                mInput.mInputFloatBuf->clear();
            }
            break;
        case InputType::INPUT_TEXT:
            if (mInput.mInputTextBuf) {
                mInput.mInputTextBuf->clear();
            }
            break;
        case InputType::INPUT_CONSTANT:
            mInput.mInputFloatConstant = 0.0f;
            break;
        case InputType::INPUT_VALUE:
            break;
        }
        mInputType = InputType::INPUT_CONSTANT;
    }

    bool NodeGraphInput::IsOfType(InputType type) {
        if (mInputType == type) {
            return true;
        }
        return false;
    }

    bool NodeGraphInput::HasInputNode() {
        if (mInputType == InputType::INPUT_NODE && mInput.mInputNode != nullptr) {
            return true;
        }
        return false;
    }

    bool NodeGraphInput::HasInputNode(NodeGraphBase* node) {
        if (mInputType == InputType::INPUT_NODE && mInput.mInputNode == node) {
            return true;
        }
        return false;
    }

    bool NodeGraphInput::HasInputValue(float* floatPtr) {
        if (mInputType == InputType::INPUT_VALUE && mInput.mInputFloat == floatPtr) {
            return true;
        }
        return false;
    }

    void NodeGraphInput::MinimizeBuffer(int32_t size) {
        if (mInputType == InputType::INPUT_NODE) {
            if (mInput.mInputNode != nullptr) {
                ASSERT(mInput.mInputNode->GetOutputSize(mInputFromOutputChannel) == size);
                //mInput.mInputNode->GetOutputOf(mInputFromOutputChannel).MinimizeBuffer(size);
            }
        }
        else if (mInputType == InputType::INPUT_ARRAY) {
            if (!mInput.mInputFloatBuf) {
                mInput.mInputFloatBuf->resize(size);
            }
        }
    }

    float& NodeGraphInput::Get(int32_t minSize, int32_t offset) {
        if (minSize > 1 && (mInputType == InputType::INPUT_CONSTANT || mInputType == InputType::INPUT_VALUE)) {
            mInputType = InputType::INPUT_ARRAY;
            mInput.mInputFloatBuf = nullptr;
        }
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutput(mInputFromOutputChannel, minSize, offset);
            }
            break;
        case InputType::INPUT_ARRAY:
            return GetArray(minSize, offset);
        case InputType::INPUT_CONSTANT:
            return mInput.mInputFloatConstant;
        case InputType::INPUT_VALUE:
            return *mInput.mInputFloat;
        case InputType::INPUT_TEXT:
            return *reinterpret_cast<float*>(mInput.mInputTextBuf->data());
        }
        return mInput.mInputFloatConstant;
    }

    float& NodeGraphInput::GetArray(int32_t minSize, int32_t offset) {
        if (mInputType == InputType::INPUT_ARRAY) {
            if (!mInput.mInputFloatBuf) {
                mInput.mInputFloatBuf = new std::vector<float>();
            }
            if (static_cast<int32_t>(mInput.mInputFloatBuf->size()) < minSize) {
                mInput.mInputFloatBuf->resize(minSize, 0.0f);
            }
            return *(mInput.mInputFloatBuf->data() + offset);
        }
        return mInput.mInputFloatConstant;
    }

    std::string_view NodeGraphInput::GetText(int32_t minSize) {
        if (mInputType == InputType::INPUT_TEXT) {
            if (!mInput.mInputTextBuf) {
                mInput.mInputTextBuf = new std::vector<char>();
                mInput.mInputTextBuf->resize(minSize + 1, 0);
                mInput.mInputTextBuf->at(0) = 0;
            }
            if (static_cast<int32_t>(mInput.mInputTextBuf->size()) < minSize + 1) {
                mInput.mInputTextBuf->resize(minSize + 1, 0);
            }
            if (!mInput.mInputTextBuf->empty()) {
                mInput.mInputTextBuf->data()[mInput.mInputTextBuf->size() - 1] = 0;
                return std::string_view(mInput.mInputTextBuf->data());
            }
        }
        else if (mInputType == InputType::INPUT_NODE) {
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutputText(mInputFromOutputChannel, minSize);
            }
        }
        return {};
    }

    NodeGraphBase* NodeGraphInput::GetInputNode() {
        if (mInputType == InputType::INPUT_NODE && mInput.mInputNode != nullptr) {
            return mInput.mInputNode;
        }
        return nullptr;
    }

    int8_t NodeGraphInput::GetInputSrcChannel() {
        if (mInputType == InputType::INPUT_NODE && mInput.mInputNode != nullptr) {
            return mInputFromOutputChannel;
        }
        return 0;
    }

    void NodeGraphInput::SetConstant(float constant) {
        if (mInputType == InputType::INPUT_ARRAY && mInput.mInputFloatBuf) {
            delete mInput.mInputFloatBuf;
            mInput.mInputFloatBuf = nullptr;
        }
        else if (mInputType == InputType::INPUT_TEXT && mInput.mInputTextBuf) {
            delete mInput.mInputTextBuf;
            mInput.mInputTextBuf = nullptr;
        }
        mInput.mInputFloatConstant = constant;
        mInputType = InputType::INPUT_CONSTANT;
        mInputFromOutputChannel = 0;
    }

    void NodeGraphInput::SetValue(float* floatPtr) {
        if (mInputType == InputType::INPUT_ARRAY && mInput.mInputFloatBuf) {
            delete mInput.mInputFloatBuf;
            mInput.mInputFloatBuf = nullptr;
        }
        else if (mInputType == InputType::INPUT_TEXT && mInput.mInputTextBuf) {
            delete mInput.mInputTextBuf;
            mInput.mInputTextBuf = nullptr;
        }
        mInput.mInputFloat = floatPtr;
        mInputType = InputType::INPUT_VALUE;
        mInputFromOutputChannel = 0;
    }

    void NodeGraphInput::SetArray(float defaultValue, int32_t minSize) {
        if (mInputType == InputType::INPUT_TEXT && mInput.mInputTextBuf) {
            delete mInput.mInputTextBuf;
            mInput.mInputTextBuf = nullptr;
        }
        if (mInputType != InputType::INPUT_ARRAY || !mInput.mInputFloatBuf) {
            mInputType = InputType::INPUT_ARRAY;
            mInput.mInputFloatBuf = new std::vector<float>();
        }
        if (static_cast<int32_t>(mInput.mInputFloatBuf->size()) < minSize) {
            mInput.mInputFloatBuf->resize(minSize, defaultValue);
        }
        mInputFromOutputChannel = 0;
    }

    void NodeGraphInput::SetText(std::string_view text) {
        if (mInputType == InputType::INPUT_ARRAY && mInput.mInputFloatBuf) {
            delete mInput.mInputFloatBuf;
            mInput.mInputFloatBuf = nullptr;
        }
        if (mInputType != InputType::INPUT_TEXT || !mInput.mInputTextBuf) {
            mInputType = InputType::INPUT_TEXT;
            mInput.mInputTextBuf = new std::vector<char>();
            mInput.mInputTextBuf->resize(text.size() + 1, 0);
            mInput.mInputTextBuf->at(0) = 0;
        }
        if (static_cast<int32_t>(mInput.mInputTextBuf->size()) < static_cast<int32_t>(text.size()) + 1) {
            mInput.mInputTextBuf->resize(text.size() + 1, 0);
        }
        memcpy(mInput.mInputTextBuf->data(), text.data(), text.size());
        mInput.mInputTextBuf->data()[text.size()] = 0;
        mInputFromOutputChannel = 0;
    }

    bool NodeGraphInput::SetInputNode(NodeGraphBase* source, int8_t sourceOutputChannel) {
        if (mInputType == InputType::INPUT_ARRAY && mInput.mInputFloatBuf) {
            delete mInput.mInputFloatBuf;
            mInput.mInputFloatBuf = nullptr;
        }
        else if (mInputType == InputType::INPUT_TEXT && mInput.mInputTextBuf) {
            delete mInput.mInputTextBuf;
            mInput.mInputTextBuf = nullptr;
        }
        if (!IsValidInOutNum(sourceOutputChannel, source->GetNumOutputs()) ||
            HasInputNode()) {
            return false;
        }

        mInput.mInputNode = source;
        mInputType = InputType::INPUT_NODE;
        mInputFromOutputChannel = sourceOutputChannel;

        return true;
    }

    NodeDataIterator NodeGraphInput::GetIterator(int32_t minSize) {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutputOf(mInputFromOutputChannel).GetIterator(minSize);
            }
            break;
        case InputType::INPUT_ARRAY:
            return NodeDataIterator(&GetArray(minSize));
        case InputType::INPUT_VALUE:
            return NodeDataIterator(mInput.mInputFloat, 0.0f);
        case InputType::INPUT_CONSTANT:
            return NodeDataIterator(&mInput.mInputFloatConstant, 0.0f);
        case InputType::INPUT_TEXT:
            return NodeDataIterator(reinterpret_cast<float*>(mInput.mInputTextBuf->data()), 0.0f);
        }
        return NodeDataIterator(&mInput.mInputFloatConstant, 0.0f);
    }

    int32_t NodeGraphInput::GetSize() {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutputSize(mInputFromOutputChannel);
            }
            break;
        case InputType::INPUT_ARRAY:
            if (mInput.mInputFloatBuf) {
                return static_cast<int32_t>(mInput.mInputFloatBuf->size());
            }
            return 1;
        case InputType::INPUT_TEXT:
            if (mInput.mInputTextBuf) {
                return static_cast<int32_t>(mInput.mInputTextBuf->size());
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
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
        case InputIterationType::SAMPLED_ARRAY:
            mInput.mIterator = input.at(mInputIndex).GetIterator(numSamples);
            break;
        case InputIterationType::SAMPLED_RWA:
            mInput.mIteratorRwa = NodeDataIteratorRwa(input.at(mInputIndex).GetIterator(numSamples));
            break;
        case InputIterationType::CONSTANT_ARRAY:
            mInput.mIterator = input.at(mInputIndex).GetIterator();
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
        case InputIterationType::SAMPLED_ARRAY:
        case InputIterationType::SAMPLED_RWA:
        case InputIterationType::CONSTANT_ARRAY:
            break;
        }
    }
}