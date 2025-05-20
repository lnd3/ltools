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
            AddConstant("Smooth", 0.5f, 1, 0.0f, 1.0f);
            AddConstant("");

            mValue = 0.0;
        }
        virtual ~GraphOutputDebug() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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
            mFilterEnvelope.SetConvergenceFactor();
        }
        virtual ~GraphOutputSpeaker() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        l::audio::AudioStream* mAudioStream;
        float mSamplesUntilUpdate = 0.0f;

        int32_t mCurrentStereoPosition;
        float mEnvelope = 0.0f;
        float mAttack = 1.0f;
        float mRelease = 1.0f;
        l::audio::FilterRWA<float> mFilterEnvelope;
    };

    /*********************************************************************/
    class GraphOutputPlot : public NodeGraphOp {
    public:
        GraphOutputPlot(NodeGraphBase* node, int32_t plotSamples) :
            NodeGraphOp(node, "Plot"),
            mInputManager(*this),
            mPlotSamples(plotSamples)
        {
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Plot", 0.0f, 1, -1.0f, 1.0f));
            AddInput2("Scale", 1, InputFlags(false, true, true, false));
            AddOutput("Plot", 0.0f, mPlotSamples, false);
        }
        virtual ~GraphOutputPlot() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        InputManager mInputManager;

        int32_t mPlotSamples = 50;
        int32_t mCurIndex = 0;
    };

    /*********************************************************************/
    class GraphOutputPCBeep : public NodeGraphOp {
    public:
        GraphOutputPCBeep(NodeGraphBase* node) :
            NodeGraphOp(node, "PC Beep")
        {
            AddInput("Trigger", 0.0f, 1, 0.0f, 1.0f);
            AddInput("Freq", 5000.0f, 1, 20.0f, 10000.0f);
            AddInput("Duration", 150.0f, 1, 1.0f, 10000.0f);
        }
        virtual ~GraphOutputPCBeep() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
		virtual void Tick(int32_t numSamples, float dt) override;
    protected:
        bool mTriggered = false;
        float mTimer = 0.0f;
    };
}

