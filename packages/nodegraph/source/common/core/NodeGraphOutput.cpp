#include "nodegraph/core/NodeGraphOutput.h"
#include "nodegraph/core/NodeGraphBase.h"

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
        int32_t lodSize = static_cast<int32_t>(size / mOutputLod);
        if (lodSize != size) {
            ASSERT(lodSize != size) << "Failed to reset 'lod' output buffer to size '" << lodSize << "' because it is already allocated for size '" << mOutputBuf->size() << "'";
        }
        if (static_cast<int32_t>(mOutputBuf->size()) < lodSize) {
            mOutputBuf->resize(lodSize);
            for (size_t i = 0; i < mOutputBuf->size(); i++) {
                (*mOutputBuf)[i] = 0.0f;
            }
        }
        mOutputPolled = true;
        return *mOutputBuf->data();
    }

    NodeDataIterator NodeGraphOutput::GetIterator(int32_t size, float lod) {
        if (mOutputLod == 1.0f && lod > 1.0f) {
            mOutputLod = lod;
        }
        float stepPerIndex = size == 1 ? 0.0f : 1.0f / mOutputLod;
        return NodeDataIterator(&Get(size), stepPerIndex);
    }

    NodeDataIterator NodeGraphOutput::GetIterator() {
        auto size = GetSize();
        float stepPerIndex = size == 1 ? 0.0f : 1.0f / mOutputLod;
        return NodeDataIterator(&Get(size), stepPerIndex);
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