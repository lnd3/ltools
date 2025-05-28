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
    class TradingFilterFlipGate : public NodeGraphOp {
    public:
        TradingFilterFlipGate(NodeGraphBase* node) :
            NodeGraphOp(node, "Flip Gate")
        {
            AddInput("In", 0.0f);
            AddInput("Pos Max Hold", 0.0f, 1, 0.0f, l::math::constants::FLTMAX);
            AddInput("Neg Max Hold", 0.0f, 1, 0.0f, l::math::constants::FLTMAX);
            AddOutput("Gate Hold", 0.0f);
            AddOutput("Gate", 0.0f);
        }

        virtual ~TradingFilterFlipGate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        bool mGate = false;
        int32_t mPosHoldCount = 0;
        int32_t mNegHoldCount = 0;
    };

    /*********************************************************************/
    class TradingFilterPulseInfo: public NodeGraphOp {
    public:
        TradingFilterPulseInfo(NodeGraphBase* node) :
            NodeGraphOp(node, "Flip Info")
        {
            AddInput("In", 0.0f);
            AddInput("Max Flips in info", 5.0f, 1, 1.0f, 100000.0f);
            AddInput("Pulse", 0.5f, 1, 0.0f, 1.0f);

            AddOutput("mean+");
            AddOutput("mean-");
            AddOutput("max+");
            AddOutput("max-");
        }

        virtual ~TradingFilterPulseInfo() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        std::vector<float> mPosPulseIntervalCount;
        std::vector<float> mNegPulseIntervalCount;
        float mValuePrev1 = 0.0f;
    };
}

