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


namespace {
}

namespace l::nodegraph {


    /*********************************************************************/
    class TradingIndicatorOBV : public NodeGraphOp {
    public:
        TradingIndicatorOBV(NodeGraphBase* node) :
            NodeGraphOp(node, "OBV Indicator")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddOutput("OBV", 0.0f);
        }

        virtual ~TradingIndicatorOBV() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = inputs.at(0).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = *input++;
                *output++ = in;
            }
        }

    protected:
    };

    /*********************************************************************/
    class TradingIndicatorRSI : public NodeGraphOp {
    public:
        TradingIndicatorRSI(NodeGraphBase* node) :
            NodeGraphOp(node, "RSI Indicator")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddOutput("RSI", 0.0f);
        }

        virtual ~TradingIndicatorRSI() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = inputs.at(0).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = *input++;
                *output++ = in;
            }
        }

    protected:
    };
}

