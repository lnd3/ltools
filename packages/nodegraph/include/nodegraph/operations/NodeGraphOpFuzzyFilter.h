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
    class FuzzyFilterFlipGate : public NodeGraphOp {
    public:
        FuzzyFilterFlipGate(NodeGraphBase* node) :
            NodeGraphOp(node, "Flip Gate")
        {
            AddInput("In", 0.0f);
            AddInput("Hold Threshold", 0.0f, 1, 0.0f, l::math::constants::FLTMAX);
            AddInput("Hold Steps", 0.0f, 1, 0.0f, l::math::constants::FLTMAX);
            AddOutput("Gate", 0.0f);
            AddOutput("Gate+Hold", 0.0f);
        }

        virtual ~FuzzyFilterFlipGate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        bool mGate = false;
        float mStrength = 0.0f;
        int32_t mTriggerCounter = 0;
        int32_t mHoldCounter = 0;
    };

    /*********************************************************************/
    class FuzzyFilterPulseInfo: public NodeGraphOp {
    public:
        FuzzyFilterPulseInfo(NodeGraphBase* node) :
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

        virtual ~FuzzyFilterPulseInfo() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        std::vector<float> mPosPulseIntervalCount;
        std::vector<float> mNegPulseIntervalCount;
        float mValuePrev1 = 0.0f;
    };
}

