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

        GraphFilterBase(NodeGraphBase* node, std::string_view name, int32_t numInputs = 0, int32_t numOutputs = 0, int32_t numConstants = 0) :
            NodeGraphOp(node, mNumDefaultInputs + numInputs, mNumDefaultOutputs + numOutputs, numConstants),
            mName(name)
        {}

        std::string defaultInStrings[mNumDefaultInputs] = { "Sync", "In", "Cutoff", "Resonance" };
        std::string defaultOutStrings[mNumDefaultOutputs] = { "Out" };

        virtual ~GraphFilterBase() = default;
        virtual void Reset() override final;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override final;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override final {
            if (inputChannel >= mNumDefaultInputs) return GetInputNameExtra(inputChannel - mNumDefaultInputs);
            if (inputChannel >= 0) return defaultInStrings[static_cast<uint8_t>(inputChannel)];
            return "";
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override final {
            if (outputChannel >= mNumDefaultOutputs) return GetOutputNameExtra(outputChannel - mNumDefaultOutputs);
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return mName;
        }

        virtual std::string_view GetInputNameExtra(int8_t) { return ""; };
        virtual std::string_view GetOutputNameExtra(int8_t) { return ""; };
        virtual void ResetInput() {};
        virtual void ResetSignal() {};
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual float ProcessSignal(float input, float cutoff, float resonance) = 0;
    protected:
        std::string mName;
        float mReset = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateSamples = 16.0f;

        float mCutoff = 0.0f;
        float mResonance = 0.0f;
        float mCutoffCPS = 0.0f; // cutoff cycles per sample
        l::audio::FilterRWA<float> mCutoffFilter;
        l::audio::FilterRWA<float> mResonanceFilter;
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
            GraphFilterBase(node, "Chamberlin two-pole", 1)
        {
            mState.resize(4);
        }

        virtual ~GraphFilterChamberlain2pole() = default;

        virtual void ResetInput() override;
        virtual void ResetSignal() override;
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) override;
        virtual float ProcessSignal(float input, float cutoff, float resonance) override;

        virtual std::string_view GetInputNameExtra(int8_t) override {
            return "Mode";
        }
    protected:
        float mInputValuePrev = 0.0f;
        float mScale = 0.0f;
        int32_t mMode = 0;

        std::vector<float> mState;
        l::audio::FilterRWA<float> mScaleFilter;
    };

}

