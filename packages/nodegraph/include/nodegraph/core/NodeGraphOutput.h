#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

namespace l::nodegraph {

    enum class OutputType {
        Default, // node will be processed if it is connected to the groups output by some route
        ExternalOutput, // node does not have meaningful output for other nodes but should still be processed (ex speaker output only has input)
        ExternalVisualOutput,
    };


    class NodeOutputDataIterator {
    public:
        NodeOutputDataIterator(float* data, int32_t size) {
            mData = data;
            mSize = size;
            mIncrement = size > 1 ? 1 : 0;
        }
        float& operator*() {
            return *mData;
        }
        float* operator++(int) {
            float* data = mData;
            mData += mIncrement;
            return data;
        }
    protected:
        float* mData = nullptr;
        int32_t mSize = 0;
        int32_t mIncrement = 0;
    };

    class NodeGraphOutput {
    public:
        NodeGraphOutput() = default;

        float mOutput = 0.0f;
        std::unique_ptr<std::vector<float>> mOutputBuf = nullptr;
        std::unique_ptr<std::string> mName = nullptr;
        bool mOutputPolled = false;

        float& Get(int32_t size = 1);
        NodeOutputDataIterator GetIterator(int32_t numSamples = 1);
        int32_t GetSize();
        bool IsPolled();
        void ResetPollState();
    };

}
