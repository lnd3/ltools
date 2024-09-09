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

    /* Mathematical operations */

    /*********************************************************************/
    class GraphSourceConstants : public NodeGraphOp {
    public:
        GraphSourceConstants(NodeGraphBase* node, int32_t mode) :
            NodeGraphOp(node, 0, 4, 4),
            mMode(mode)
        {}

        virtual ~GraphSourceConstants() = default;
        virtual void Reset();
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;
        virtual std::string_view GetName() override {
            switch (mMode) {
            case 0:
                return "Constant [0,1]";
            case 1:
                return "Constant [-1,1]";
            case 2:
                return "Constant [0,100]";
            case 3:
                return "Constant [-inf,inf]";
            };
            return "";
        }
        virtual bool IsDataVisible(int8_t) override {return true;}
        virtual bool IsDataEditable(int8_t) override {return true;}
    protected:
        int32_t mMode;
        float mMax = 1.0f;
        float mMin = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceTime : public NodeGraphOp {
    public:
        GraphSourceTime(NodeGraphBase* node) :
            NodeGraphOp(node, 0, 2, 0)
        {}

        std::string defaultOutStrings[2] = { "Audio Time", "Frame Time"};

        virtual ~GraphSourceTime() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Time";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    protected:
        float mAudioTime = 0.0f;
        float mFrameTime = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceSine : public NodeGraphOp {
    public:
        GraphSourceSine(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 1)
        {}

        std::string defaultInStrings[5] = { "Freq", "Volume", "Fmod", "Phase", "Reset"};
        std::string defaultOutStrings[1] = { "Out"};

        virtual ~GraphSourceSine() = default;
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
    class GraphSourceSineFM : public NodeGraphOp {
    public:
        GraphSourceSineFM(NodeGraphBase* node) :
            NodeGraphOp(node, 8, 1)
        {}

        std::string defaultInStrings[8] = { "Freq", "Volume", "Fmod", "FmodFreq", "FmodVol", "FmodOfs", "FmodGain", "Reset"};
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSourceSineFM() = default;
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
    class GraphSourceSineFM2 : public NodeGraphOp {
    public:
        GraphSourceSineFM2(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 1)
        {}

        std::string defaultInStrings[5] = { "Freq", "Volume", "FmodVol", "FmodFreq", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSourceSineFM2() = default;
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
    class GraphSourceSineFM3 : public NodeGraphOp {
    public:
        GraphSourceSineFM3(NodeGraphBase* node) :
            NodeGraphOp(node, 4, 1)
        {}

        std::string defaultInStrings[4] = { "Freq", "Volume", "Fmod", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSourceSineFM3() = default;
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
    class GraphSourceSaw : public NodeGraphOp {
    public:
        GraphSourceSaw(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 1)
        {}

        std::string defaultInStrings[5] = { "Freq", "Volume", "Fmod", "Phase", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSourceSaw() = default;
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

