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

    class GraphSignalBase : public NodeGraphOp {
    public:

        static const int8_t mNumDefaultInputs = 7;
        static const int8_t mNumDefaultOutputs = 1;

        GraphSignalBase(NodeGraphBase* node, std::string_view name, int32_t numInputs = 0, int32_t numOutputs = 0, int32_t numConstants = 0) :
            NodeGraphOp(node, mNumDefaultInputs + numInputs, mNumDefaultOutputs + numOutputs, numConstants),
            mName(name)
        {}

        std::string defaultInStrings[mNumDefaultInputs] = { "Reset", "Freq", "Volume", "Smooth", "Cutoff", "Resonance", "Phase expansion"};
        std::string defaultOutStrings[mNumDefaultOutputs] = { "Out" };

        virtual ~GraphSignalBase() = default;
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
        virtual void ResetSignal() {};
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual float GenerateSignal(float deltaTime, float freq, float deltaPhase) = 0;
    protected:
        std::string mName;

        float mReset = 0.0f;
        float mFreq = 0.0f;
        float mVolume = 0.0f;
        float mSmooth = 0.5f;
        float mSignal = 0.0f;
        float mWave = 0.0f;
        float mDeltaPhase = 0.0f;
        float mDeltaTime = 0.0f;
        float mVolumeTarget = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateSamples = 256.0f;

        // high pass
        float mHPCutoff = 0.5f;
        float mHPResonance = 0.0001f;
        float mHPState0 = 0.0f;
        float mHPState1 = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSine2 : public GraphSignalBase {
    public:
        GraphSignalSine2(NodeGraphBase* node) :
            GraphSignalBase(node, "Sine 2", 2)
        {}
        std::string extraString[2] = { "Fmod", "Phase" };

        virtual ~GraphSignalSine2() = default;
        virtual std::string_view GetInputNameExtra(int8_t extraInputChannel) override {
            if(extraInputChannel < 2) return extraString[static_cast<uint8_t>(extraInputChannel)];
            return "";
        }
        void ResetSignal() override;
        void UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        float GenerateSignal(float deltaTime, float freq, float deltaPhase) override;
    protected:
        float mFmod = 0.0f;
        float mPmod = 0.0f;
        float mPhase = 0.0f;
        float mPhaseFmod = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSine : public NodeGraphOp {
    public:
        GraphSignalSine(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 1)
        {}

        std::string defaultInStrings[6] = { "Freq", "Volume", "Fmod", "Phase", "Smooth", "Reset"};
        std::string defaultOutStrings[1] = { "Out"};

        virtual ~GraphSignalSine() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;
        double mPhaseFmod = 0.0;

        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSineFM : public NodeGraphOp {
    public:
        GraphSignalSineFM(NodeGraphBase* node) :
            NodeGraphOp(node, 9, 1)
        {}

        std::string defaultInStrings[9] = { "Freq", "Volume", "Fmod", "FmodFreq", "FmodVol", "FmodOfs", "FmodGain", "Smooth", "Reset"};
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSineFM() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine FM 1";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;


        double mPhaseFmod = 0.0;
        double mFmodFrq = 0.0;
        double mFmodVol = 0.0;
        double mFmodOfs = 0.0;

        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSineFM2 : public NodeGraphOp {
    public:
        GraphSignalSineFM2(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 1)
        {}

        std::string defaultInStrings[6] = { "Freq", "Volume", "FmodVol", "FmodFreq", "Smooth", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSineFM2() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine FM 2";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        double mDeltaLimit = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;
        double mPhaseFmod = 0.0;
        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSineFM3 : public NodeGraphOp {
    public:
        GraphSignalSineFM3(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 1)
        {}

        std::string defaultInStrings[5] = { "Freq", "Volume", "Fmod", "Smooth", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSineFM3() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine FM 3";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mPhaseFmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        double mDeltaLimit = 0.0;
        float mVol = 0.0f;
        double mPhase = 0.0;
        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSaw : public NodeGraphOp {
    public:
        GraphSignalSaw(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 1)
        {}

        std::string defaultInStrings[6] = { "Freq", "Volume", "Fmod", "Phase", "Smooth", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSaw() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Saw";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mDeltaTime = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;
        double mPhaseFmod = 0.0;
        double mWave = 0.0;
        float mSamplesUntilUpdate = 0.0f;
    };
}

