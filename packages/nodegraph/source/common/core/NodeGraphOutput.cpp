#include "nodegraph/core/NodeGraphOutput.h"
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {
    void NodeGraphOutput::Reset() {
        if (mOutputBuf != nullptr) {
            delete mOutputBuf;
            mOutputBuf = nullptr;
        }
    }

    void NodeGraphOutput::Clear() {
        if ( mOutputBuf != nullptr) {
            mOutputBuf->clear();
        }
    }

    void NodeGraphOutput::MinimizeBuffer(int32_t size) {
        if (mOutputBuf != nullptr) {
            int32_t lodSize = static_cast<int32_t>(size / mOutputLod);
            if (static_cast<int32_t>(mOutputBuf->size()) != lodSize) {
                mOutputBuf->resize(lodSize);
            }
        }
    }

    float& NodeGraphOutput::Get(int32_t minSize, int32_t offset) {
        if (mOutputBuf == nullptr) {
            if (minSize <= 1) {
                mOutputPolled = true;
                return mOutput;
            }
            else {
                mOutputBuf = new std::vector<float>();
            }
        }
        int32_t lodSize = static_cast<int32_t>(minSize / mOutputLod);
        if (lodSize != minSize) {
            ASSERT(lodSize != minSize) << "Failed to reset 'lod' output buffer to size '" << lodSize << "' because it is already allocated for size '" << mOutputBuf->size() << "'";
        }
        if (static_cast<int32_t>(mOutputBuf->size()) < lodSize) {
            mOutputBuf->resize(lodSize);
            for (size_t i = 0; i < mOutputBuf->size(); i++) {
                (*mOutputBuf)[i] = 0.0f;
            }
        }
        mOutputPolled = true;
        return *(mOutputBuf->data() + offset);
    }

    std::string_view NodeGraphOutput::GetText(int32_t minSize) {
        auto out = reinterpret_cast<char*>(&Get(1 + minSize / 4));
        return std::string_view(out);
    }

    void NodeGraphOutput::SetText(std::string_view text) {
        auto p = reinterpret_cast<char*>(&Get(1 + static_cast<int32_t>(text.size())));
        memcpy(p, text.data(), text.size());
        *(p + text.size()) = 0;
    }

    NodeDataIterator NodeGraphOutput::GetIterator(int32_t minSize, float lod) {
        if (lod >= 1.0f && lod <= minSize) {
            mOutputLod = lod;
        }
        float stepPerIndex = minSize == 1 ? 0.0f : 1.0f / mOutputLod;
        return NodeDataIterator(&Get(minSize), stepPerIndex);
    }

    NodeDataIterator NodeGraphOutput::GetIterator() {
        auto size = GetSize();
        float stepPerIndex = size == 1 ? 0.0f : 1.0f / mOutputLod;
        return NodeDataIterator(&Get(size), stepPerIndex);
    }

    int32_t NodeGraphOutput::GetSize() {
        if (mOutputBuf != nullptr) {
            return static_cast<int32_t>(mOutputBuf->size());
        }
        return 1;
    }

    bool NodeGraphOutput::IsPolled() {
        return mOutputPolled;
    }

    void NodeGraphOutput::ResetPollState() {
        mOutputPolled = false;
    }
}