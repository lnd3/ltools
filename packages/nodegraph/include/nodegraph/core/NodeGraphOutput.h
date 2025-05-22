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
        bool mOutputPolled = false;

        void Clear();
        float& Get(int32_t minSize = 1, int32_t offset = 0);
        std::string_view GetText(int32_t minSize = 16);
        void SetText(std::string_view text);
        NodeDataIterator GetIterator(int32_t minSize, float lod = 1.0f);
        NodeDataIterator GetIterator();
        int32_t GetSize();
        bool IsPolled();
        void ResetPollState();
    };

}

