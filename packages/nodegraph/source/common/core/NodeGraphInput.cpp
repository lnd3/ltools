#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {
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

    float NodeGraphInput::Get() {
        float value = 0.0f;
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                value = mInput.mInputNode->GetOutput(mInputFromOutputChannel);
            }
            break;
        case InputType::INPUT_CONSTANT:
            value = mInput.mInputFloatConstant;
            break;
        case InputType::INPUT_ARRAY:
            return *mInputBuf->data();
        case InputType::INPUT_VALUE:
            value = *mInput.mInputFloat;
            break;
        case InputType::INPUT_EMPTY:
            break;
        }
        return l::math::functions::clamp(value, mBoundMin, mBoundMax);
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

}