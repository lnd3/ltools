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
    class GraphDataBusDataIn : public NodeGraphOp2 {
    public:
        GraphDataBusDataIn(NodeGraphBase* node, int32_t inputDataStride) :
            NodeGraphOp2(node, "Bus Data In x6"),
            mInputDataStride(inputDataStride)
        {
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Bus Data", 0.0f, 2));

            for (int32_t i = 0; i < mInputDataStride; i++) {
                AddOutput("Out " + std::to_string(i), 0.0f, 2);
            }
        }
        virtual ~GraphDataBusDataIn() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mInputDataStride = 1;
    };

    /*********************************************************************/
    class GraphDataBusDataOut : public NodeGraphOp2 {
    public:
        GraphDataBusDataOut(NodeGraphBase* node, int32_t outputDataStride) :
            NodeGraphOp2(node, "Bus Data Out x6"),
            mOutputDataStride(outputDataStride)
        {
            for (int32_t i = 0; i < mOutputDataStride; i++) {
                mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("In " + std::to_string(i), 0.0f, 2));
            }
            AddOutput("Bus Data", 0.0f, 2);
        }
        virtual ~GraphDataBusDataOut() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mOutputDataStride = 1;
    };

}

