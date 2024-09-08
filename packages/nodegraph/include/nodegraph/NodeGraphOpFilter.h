#pragma once
#include "nodegraph/NodeGraph.h"

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

    /* Stateful filtering operations */

    /*********************************************************************/
    class GraphFilterLowpass : public NodeGraphOp {
    public:
        std::string defaultInStrings[3] = { "Data", "Cutoff", "Resonance"};
        std::string defaultOutStrings[1] = { "Out" };

        GraphFilterLowpass(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        virtual ~GraphFilterLowpass() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t channel) override { return channel > 0 ? true : false; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }

        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }

        virtual std::string_view GetName() override {
            return "Lowpass";
        }
    protected:
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };

    /*********************************************************************/
    class GraphFilterEnvelope : public NodeGraphOp {
    public:
        std::string defaultInStrings[5] = { "Freq", "Velocity", "Attack", "Release", "Fade"};
        std::string defaultOutStrings[2] = { "Freq", "Volume"};

        GraphFilterEnvelope(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 2)
        {}

        virtual ~GraphFilterEnvelope() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) {}
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual bool IsDataVisible(int8_t channel) override {
            return channel >= 1 ? true : false;
        }
        virtual bool IsDataEditable(int8_t channel) override {
            return channel >= 1 ? true : false;
        }
        virtual std::string_view GetName() override {
            return "Envelope";
        }
    protected:
        int32_t mFrameCount = 0;
        float mEnvelopeTarget = 0.0f;
        float mFreq = 0.0f;
        float mEnvelope = 0.0f;
    };

}

