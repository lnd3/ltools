#include "nodegraph/core/NodeGraphOutput.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {
    float& NodeGraphOutput::Get(int32_t size) {
        if (!mOutputBuf) {
            if (size <= 1) {
                mOutputPolled = true;
                return mOutput;
            }
            else {
                mOutputBuf = std::make_unique<std::vector<float>>();
            }
        }
        if (static_cast<int32_t>(mOutputBuf->size()) < size) {
            mOutputBuf->resize(size);
        }
        mOutputPolled = true;
        return *mOutputBuf->data();
    }

    int32_t NodeGraphOutput::GetSize() {
        if (!mOutputBuf) {
            return 1;
        }
        else {
            return static_cast<int32_t>(mOutputBuf->size());
        }
    }

    bool NodeGraphOutput::IsPolled() {
        return mOutputPolled;
    }

    void NodeGraphOutput::ResetPollState() {
        mOutputPolled = false;
    }

}