#pragma once
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/LoggingAll.h"

#include "hid/KeyboardPiano.h"
#include "hid/Midi.h"

#include "audio/PortAudio.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <math.h>
#include <random>
#include <unordered_set>

namespace l::nodegraph {

    /*********************************************************************/
    class DataIODataIn : public NodeGraphOp {
    public:
        DataIODataIn(NodeGraphBase* node, int32_t inputDataStride) :
            NodeGraphOp(node, "Bus Data In x6"),
            mInputManager(*this),
            mInputDataStride(inputDataStride)
        {
            mInputManager.AddInput(InputIterationType::SAMPLED_ARRAY, AddInput2("Bus Data", 2, InputFlags(false, false, false, false)));

            for (int32_t i = 0; i < mInputDataStride; i++) {
                AddOutput("Out " + std::to_string(i), 0.0f, 2, false);
            }
        }
        virtual ~DataIODataIn() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;

        int32_t mInputDataStride = 1;
    };

    /*********************************************************************/
    class DataIODataOut : public NodeGraphOp {
    public:
        DataIODataOut(NodeGraphBase* node, int32_t outputDataStride) :
            NodeGraphOp(node, "Bus Data Out x6"),
            mInputManager(*this),
            mOutputDataStride(outputDataStride)
        {
            for (int32_t i = 0; i < mOutputDataStride; i++) {
                mInputManager.AddInput(InputIterationType::SAMPLED_ARRAY, AddInput2("In " + std::to_string(i), 2, InputFlags(false, false, false, false)));
            }
            AddOutput("Bus Data", 0.0f, 2, false);
        }
        virtual ~DataIODataOut() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;

        int32_t mOutputDataStride = 1;
    };

    /*********************************************************************/
    class GraphCache : public NodeGraphOpCached {
    public:
        GraphCache(NodeGraphBase* node, int32_t channels) :
            NodeGraphOpCached(node, "Data Buffer"),
            mChannels(channels)
        {
            for (int32_t i = 0; i < mChannels; i++) {
                auto id = std::to_string(i);
                AddInput2("In" + id, 2, InputFlags(false, false, false, false));
                AddOutput2("Out " + id, 2, OutputFlags(false, false));
            }
        }
        virtual ~GraphCache() = default;

        virtual void ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void ProcessReadCached(int32_t readSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mChannels = 1;
        std::vector<float> mBuffer;
    };

}

