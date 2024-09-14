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
    class GraphOutputDebug : public NodeGraphOp {
    public:
        GraphOutputDebug(NodeGraphBase* node) :
            NodeGraphOp(node, "Debug")
        {
            AddInput("Debug");
            AddConstant("Smooth", 0.5f, 0.0f, 1.0f);
            AddConstant("");

            mValue = 0.0;
        }
        virtual ~GraphOutputDebug() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        float mValue = 0.0F;
    };

    /*********************************************************************/
    class GraphOutputSpeaker : public NodeGraphOp {
    public:
        GraphOutputSpeaker(NodeGraphBase* node, l::audio::AudioStream* stream = nullptr) :
            NodeGraphOp(node, "Speaker"),
            mAudioStream(stream),
            mCurrentStereoPosition(0)
        {
            AddInput("Left");
            AddInput("Right");
            AddInput("Volume", 0.5f, 1, 0.0f, 1.0f);

            mEnvelope = 0.0f;
            mFilterEnvelope.SetConvergence();
        }
        virtual ~GraphOutputSpeaker() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        l::audio::AudioStream* mAudioStream;
        int32_t mCurrentStereoPosition;
        float mEnvelope = 0.0f;
        l::audio::FilterRWA<float> mFilterEnvelope;
    };

    /*********************************************************************/
    class GraphOutputPlot : public NodeGraphOp {
    public:
        GraphOutputPlot(NodeGraphBase* node, int32_t plotSamples) :
            NodeGraphOp(node, "Plot"),
            mPlotSamples(plotSamples)
        {
            AddInput("Plot", 0.0f, 1, -1.0f, 1.0f);
            AddOutput("Plot", 0.0f, mPlotSamples);
        }
        virtual ~GraphOutputPlot() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mPlotSamples = 50;
        int32_t mCurIndex = 0;
    };

}

