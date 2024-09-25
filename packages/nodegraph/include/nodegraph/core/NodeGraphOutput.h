#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

#include "nodegraph/core/NodeGraphData.h"

namespace l::nodegraph {

    class NodeGraphOutput {
    public:
        NodeGraphOutput() = default;

        float mOutput = 0.0f;
        float mOutputLod = 1.0f; // buffer size level of detail  value[1.0f, buffer size] (if 1 it will write all generated values to the buffer, if 'buffer size' it will only have the latest written value),
        std::unique_ptr<std::vector<float>> mOutputBuf = nullptr;
        std::unique_ptr<std::string> mName = nullptr;
        bool mOutputPolled = false;

        float& Get(int32_t size = 1);
        NodeDataIterator GetIterator(int32_t size, float lod = 1.0f);
        NodeDataIterator GetIterator();
        int32_t GetSize();
        bool IsPolled();
        void ResetPollState();
    };

}

