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
    class GraphDataBusDataIn : public NodeGraphOp {
    public:
        GraphDataBusDataIn(NodeGraphBase* node, int32_t inputDataStride) :
            NodeGraphOp(node, "Bus Data In x6"),
            mInputManager(*this),
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
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;

        int32_t mInputDataStride = 1;
    };

    /*********************************************************************/
    class GraphDataBusDataOut : public NodeGraphOp {
    public:
        GraphDataBusDataOut(NodeGraphBase* node, int32_t outputDataStride) :
            NodeGraphOp(node, "Bus Data Out x6"),
            mInputManager(*this),
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
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;

        int32_t mOutputDataStride = 1;
    };

    /*********************************************************************/
    class GraphDataCandleStickDataIn : public NodeGraphOp {
    public:
        GraphDataCandleStickDataIn(NodeGraphBase* node) :
            NodeGraphOp(node, "Candle Stick Data In")
        {
            AddInput("In", 0.0f, 2, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Interval", 8.0f, 1, 0.0f, 10.0f);

            AddOutput("Out (interleaved)", 0.0f, 2);
            AddOutput("Open", 0.0f, 2);
            AddOutput("Close", 0.0f, 2);
            AddOutput("High", 0.0f, 2);
            AddOutput("Low", 0.0f, 2);
            AddOutput("Volume", 0.0f, 2);
            AddOutput("Unixtime", 0.0f, 2);
        }
        virtual ~GraphDataCandleStickDataIn() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mInputReadPos = 0;
        int32_t mInputWritePos = 0;
    };

    /*********************************************************************/
    class GraphDataTradeSignal: public NodeGraphOp {
    public:
        GraphDataTradeSignal(NodeGraphBase* node) :
            NodeGraphOp(node, "Candle Stick Data In")
        {
            AddInput("Trade Signal", 0.0f, 2, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddOutput("Out", 0.0f, 2, false);
        }
        virtual ~GraphDataTradeSignal() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
    };

    /*********************************************************************/
    class GraphDataInputBuffer : public NodeGraphOp {
    public:
        GraphDataInputBuffer(NodeGraphBase* node, int32_t inputStride) :
            NodeGraphOp(node, "Data Buffer"),
            mInputStride(inputStride)
        {
            AddInput("In", 0.0f, 2, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            for (int32_t i = 0; i < mInputStride; i++) {
                AddOutput("Out " + std::to_string(i), 0.0f, 2);
            }
        }
        virtual ~GraphDataInputBuffer() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mInputStride = 1;
        int32_t mInputReadPos = 0;
        int32_t mInputWritePos = 0;
    };

}

