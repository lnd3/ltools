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

    /* Stateful filtering operations */
    class GraphFilterBase : public NodeGraphOp {
    public:

        static const int8_t mNumDefaultInputs = 4;
        static const int8_t mNumDefaultOutputs = 1;

        GraphFilterBase(NodeGraphBase* node, std::string_view name) :
            NodeGraphOp(node, name),
            mNodeInputManager(*this)
        {
            mDefaultInStrings.clear();
            mDefaultOutStrings.clear();
            mDefaultInData.clear();

            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Sync", 0.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("In"));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED_RWA, AddInput("Cutoff", 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED_RWA, AddInput("Resonance", 0.0f));

            AddOutput("Out", 0.0f);
        }

        virtual ~GraphFilterBase() = default;
        virtual void Reset() override final;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override final;

        virtual void ResetInput() {};
        virtual void ResetSignal() {};
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual float ProcessSignal(float input, float cutoff, float resonance) = 0;
    protected:
        NodeInputManager mNodeInputManager;

        float mSync = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateRate = 128.0f;
    };

    /*********************************************************************/
    class GraphFilterLowpass : public GraphFilterBase {
    public:
        GraphFilterLowpass(NodeGraphBase* node) :
            GraphFilterBase(node, "Lowpass")
        {}
        virtual ~GraphFilterLowpass() = default;

        virtual void ResetInput() override;
        virtual void ResetSignal() override;
        virtual float ProcessSignal(float input, float cutoff, float resonance) override;
    protected:
        float mInputValuePrev = 0.0f;
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };

    /*********************************************************************/
    class GraphFilterHighpass : public GraphFilterBase {
    public:
        GraphFilterHighpass(NodeGraphBase* node) :
            GraphFilterBase(node, "Highpass")
        {}
        virtual ~GraphFilterHighpass() = default;

        virtual void ResetInput() override;
        virtual void ResetSignal() override;
        virtual float ProcessSignal(float input, float cutoff, float resonance) override;
    protected:
        float mInputValuePrev = 0.0f;
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };

    /*********************************************************************/
    // source: https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
    class GraphFilterChamberlain2pole : public GraphFilterBase {
    public:
        GraphFilterChamberlain2pole(NodeGraphBase* node) :
            GraphFilterBase(node, "Chamberlin two-pole")
        {
            AddInput("Mode");
            mState.resize(4);
        }
        virtual ~GraphFilterChamberlain2pole() = default;

        virtual void ResetInput() override;
        virtual void ResetSignal() override;
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) override;
        virtual float ProcessSignal(float input, float cutoff, float resonance) override;
    protected:
        float mInputValuePrev = 0.0f;
        float mScale = 0.0f;
        int32_t mMode = 0;

        std::vector<float> mState;
        l::audio::FilterRWA<float> mScaleFilter;
    };

}

