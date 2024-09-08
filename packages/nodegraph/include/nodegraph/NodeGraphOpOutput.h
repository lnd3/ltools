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

    /*********************************************************************/
    class GraphOutputDebug : public NodeGraphOp {
    public:
        std::string defaultInStrings[2] = { "Debug", "Smooth" };
        GraphOutputDebug(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 0, 2)
        {}
        virtual ~GraphOutputDebug() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        virtual bool IsDataEditable(int8_t channel) override {
            return channel == 1;
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultInStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Debug";
        }
    protected:
        float mValue = 0.0F;
    };

    /*********************************************************************/
    class GraphOutputSpeaker : public NodeGraphOp {
    public:
        std::string defaultInStrings[3] = { "Left", "Right", "Volume"};

        GraphOutputSpeaker(NodeGraphBase* node, l::audio::AudioStream* stream = nullptr) :
            NodeGraphOp(node, 3, 0, 0),
            mAudioStream(stream),
            mCurrentStereoPosition(0)
        {}
        virtual ~GraphOutputSpeaker() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        virtual bool IsDataEditable(int8_t channel) override {
            return channel == 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t outputChannel) override {
            return defaultInStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Speaker";
        }

    protected:
        l::audio::AudioStream* mAudioStream;
        int32_t mCurrentStereoPosition;
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphOutputPlot : public NodeGraphOp {
    public:
        std::string defaultInStrings[2] = { "Plot", "Smooth" };
        GraphOutputPlot(NodeGraphBase* node, int32_t plotSamples) :
            NodeGraphOp(node, 1, 1, 0),
            mPlotSamples(plotSamples)
        {
            mNode->GetOutput(0, mPlotSamples);
        }
        virtual ~GraphOutputPlot() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultInStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Plot";
        }
    protected:
        int32_t mPlotSamples = 50;
        int32_t mCurIndex = 0;
    };

}

