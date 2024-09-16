#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    std::pair<float, float> GetInputBounds(InputBound bound) {
        switch (bound) {
        case InputBound::INPUT_0_TO_1:
            return { 0.0f, 1.0f };
        case InputBound::INPUT_0_TO_2:
            return { 0.0f, 2.0f };
        case InputBound::INPUT_NEG_1_POS_1:
            return { -1.0f, 1.0f };
        case InputBound::INPUT_0_100:
            return { 0.0f, 100.0f };
        case InputBound::INPUT_UNBOUNDED:
            return { -l::math::constants::FLTMAX, l::math::constants::FLTMAX };
        case InputBound::INPUT_CUSTOM:
            return { 0.0f, 0.0f };
        }
        return { 0.0f, 0.0f };
    }

    void NodeGraphInput::Reset() {
        if (mInputType == InputType::INPUT_NODE || mInputType == InputType::INPUT_VALUE) {
            mInput.mInputNode = nullptr;
            mInputType = InputType::INPUT_EMPTY;
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
        case InputType::INPUT_EMPTY:
            break;
        }
        return mInput.mInputFloatConstant;
    }

    NodeInputDataIterator NodeGraphInput::GetBufferIterator(int32_t numSamples) {
        auto size = GetSize();
        if (size > 1) {
            ASSERT(size == numSamples);
        }
        return NodeInputDataIterator(&Get(size), size);
    }

    NodeInputDataIterator NodeGraphInput::GetArrayIterator() {
        auto size = GetSize();
        return NodeInputDataIterator(&Get(size), size);
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
        case InputType::INPUT_EMPTY:
            break;
        }
        return 1;
    }
}