#pragma once
#include "nodegraph/NodeGraph.h"

#include "logging/LoggingAll.h"

#include "hid/KeyboardPiano.h"
#include "hid/Midi.h"

#include "audio/PortAudio.h"

#include "math/MathFunc.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <math.h>
#include <random>

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

        std::string defaultInStrings[5] = { "Note", "Volume", "Fmod", "Phase", "Reset"};
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
            return "Sine FM";
        }
    protected:
        double mNote = 0.0f;
        float mVolume = 0.0f;
        double mFmod = 0.0f;
        double mPmod = 0.0f;
        float mReset = 0.0f;

        double mWave = 0.0f;
        double mDeltaTime = 0.0f;
        float mVol = 0.0f;

        double mPhase = 0.0f;
        double mPhaseMod = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceSineFM : public NodeGraphOp {
    public:
        GraphSourceSineFM(NodeGraphBase* node) :
            NodeGraphOp(node, 8, 1)
        {}

        std::string defaultInStrings[8] = { "Note", "Volume", "Fmod", "FmodFreq", "FmodVol", "FmodOfs", "FmodGain", "Reset"};
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
            return "Sine FM";
        }
    protected:
        double mNote = 0.0;
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

        std::string defaultInStrings[5] = { "Note", "Volume", "FmodVol", "FmodFreq", "Reset" };
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
        double mNote = 0.0;
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

        std::string defaultInStrings[4] = { "Note", "Volume", "Fmod", "Reset" };
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
        double mNote = 0.0;
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
    class GraphNumericAdd : public NodeGraphOp {
    public:
        GraphNumericAdd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericAdd() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() + inputs.at(1).Get();
        }
        virtual std::string_view GetName() override {
            return "Add";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphNumericMultiply() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get();
        }
        virtual std::string_view GetName() override {
            return "Multiply";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericSubtract() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() - inputs.at(1).Get();
        }
        virtual std::string_view GetName() override {
            return "Subtract";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericNegate : public NodeGraphOp {
    public:
        GraphNumericNegate(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericNegate() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = -inputs.at(0).Get();
        }
        virtual std::string_view GetName() override {
            return "Negate";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericIntegral() = default;
        virtual void Reset() override {
            mOutput = 0.0f;
        }

        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            mOutput += inputs.at(0).Get();
            outputs.at(0).mOutput = mOutput;
        }
        virtual std::string_view GetName() override {
            return "Integral";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    protected:
        float mOutput = 0.0f;
    };

    /*********************************************************************/
    class GraphNumericMultiply3 : public NodeGraphOp {
    public:
        GraphNumericMultiply3(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        virtual ~GraphNumericMultiply3() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get() * inputs.at(2).Get();
        }
        virtual std::string_view GetName() override {
            return "Multiply3";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericMultiplyAndAdd : public NodeGraphOp {
    public:
        GraphNumericMultiplyAndAdd(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        std::string defaultInStrings[3] = { "Factor 1", "Factor 2", "Term 1" };

        virtual ~GraphNumericMultiplyAndAdd() = default;
        void virtual Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get() + inputs.at(2).Get();
        }
        virtual std::string_view GetName() override {
            return "Multiply & Add";
        }

        virtual std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericRound : public NodeGraphOp {
    public:
        GraphNumericRound(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}
        virtual ~GraphNumericRound() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = l::math::functions::round(inputs.at(0).Get());
        }
        virtual std::string_view GetName() override {
            return "Round";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /* Logical operations */

    /*********************************************************************/
    class GraphLogicalAnd : public NodeGraphOp {
    public:
        GraphLogicalAnd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalAnd() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            bool input1 = inputs.at(0).Get() != 0.0f;
            bool input2 = inputs.at(1).Get() != 0.0f;
            outputs.at(0).mOutput = (input1 && input2) ? 1.0f : 0.0f;
        }
        std::string_view GetName() override {
            return "And";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalOr() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            bool input1 = inputs.at(0).Get() != 0.0f;
            bool input2 = inputs.at(1).Get() != 0.0f;
            outputs.at(0).mOutput = (input1 || input2) ? 1.0f : 0.0f;
        }
        std::string_view GetName() override {
            return "Or";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalXor() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            bool input1 = inputs.at(0).Get() != 0.0f;
            bool input2 = inputs.at(1).Get() != 0.0f;
            outputs.at(0).mOutput = (input1 ^ input2) ? 1.0f : 0.0f;
        }
        virtual std::string_view GetName() override {
            return "Xor";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

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
        std::string defaultInStrings[5] = { "Note", "Velocity", "Attack", "Release", "Fade"};
        std::string defaultOutStrings[2] = { "Note", "Volume"};

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
        uint32_t mFrameCount = 0;
        float mEnvelopeTarget = 0.0f;
        float mNote = 0.0f;
        float mEnvelope = 0.0f;
    };

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

    /*********************************************************************/
    class GraphEffectReverb1 : public NodeGraphOp {
    public:
        std::string defaultInStrings[11] = { "In 1", "In 2", "Mix", "Attenuation", "Room Size", "Delay 1", "Feedback 1", "Delay 2", "Feedback 2", "Delay 3", "Feedback 3" };
        std::string defaultOutStrings[2] = { "Out 1", "Out 2" };
        
        uint32_t GetFramesPerRoomSize(float roomSize) {
            const float metersPerFrame = 334.0f / 44100.0f; // (m/s)/(frames/s) = m/frames;
            const float metersToWallPerFrame = metersPerFrame / 2.0f; // half the distance to wall for the bounced distance
            const float framesPerRoom = roomSize / metersToWallPerFrame;
            return static_cast<uint32_t>(framesPerRoom);
        }

        const float maxRoomSizeInMeters = 334.0f; // 334 meters large is 2 seconds of reverbation

        GraphEffectReverb1(NodeGraphBase * node) :
            NodeGraphOp(node, 11, 2, 0) {
                uint32_t bufferSize = GetFramesPerRoomSize(maxRoomSizeInMeters);
                buf0.resize(bufferSize);
                buf1.resize(bufferSize);
            }
            
        virtual ~GraphEffectReverb1() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>&inputs, std::vector<NodeGraphOutput>&outputs) override;
        virtual bool IsDataVisible(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual bool IsDataEditable(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Reverb 1";
        }

    protected:
        bool mInited = false;
        std::vector<float> buf0;
        std::vector<float> buf1;
        uint32_t bufIndex = 0;
        float mix = 0.0f;
        float fb = 0.0f;
        float fb0 = 0.0f;
        float fb1 = 0.0f;
        float fb2 = 0.0f;
        float d0 = 0.0f;
        float d1 = 0.0f;
        float d2 = 0.0f;
        
    };

    /*********************************************************************/
    class GraphEffectReverb2 : public NodeGraphOp {
    public:
        std::string defaultInStrings[12] = { "In 1", "In 2", "Mix", "Feedback", "Room Size", "Width", "First tap", "Longest tap", "Num taps", "Tap bulge", "Filter cutoff", "Filter res"};
        std::string defaultOutStrings[4] = { "Rev 1", "Rev 2", "Tap 1", "Tap 2"};

        uint32_t GetFramesPerRoomSize(float roomSize) {
            const float metersPerFrame = 334.0f / 44100.0f; // (m/s)/(frames/s) = m/frames;
            const float metersToWallPerFrame = metersPerFrame / 2.0f; // half the distance to wall for the bounced distance
            const float framesPerRoom = roomSize / metersToWallPerFrame;
            return static_cast<uint32_t>(framesPerRoom);
        }

        const float maxRoomSizeInMeters = 334.0f; // 334 meters large is 2 seconds of reverbation

        GraphEffectReverb2(NodeGraphBase* node) :
            NodeGraphOp(node, 12, 4, 0)
        {
            uint32_t bufferSize = GetFramesPerRoomSize(maxRoomSizeInMeters);
            bufRev0.resize(bufferSize);
            bufRev1.resize(bufferSize);
            bufEarlyTap0.resize(bufferSize);
            bufEarlyTap1.resize(bufferSize);
        }

        virtual ~GraphEffectReverb2() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual bool IsDataEditable(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Reverb 2";
        }

    protected:
        bool mInited = false;

        std::vector<float> bufRev0;
        std::vector<float> bufRev1;
        std::vector<float> bufEarlyTap0;
        std::vector<float> bufEarlyTap1;
        uint32_t bufIndex = 0;
        uint32_t bufTapIndex = 0;

        std::minstd_rand mLCG;

        float mLP0 = 0.0f;
        float mLP1 = 0.0f;
        float mLP2 = 0.0f;
        float mLP3 = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectLimiter : public NodeGraphOp {
    public:
        std::string defaultInStrings[6] = { "In 1", "In 2", "Attack", "Release", "Preamp", "Limit" };
        std::string defaultOutStrings[3] = { "Out 1", "Out 2", "Envelope"};

        GraphEffectLimiter(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 3, 0)
        {}

        virtual ~GraphEffectLimiter() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual bool IsDataEditable(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Limiter";
        }

    protected:
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectEnvelopeFollower : public NodeGraphOp {
    public:
        std::string defaultInStrings[6] = { "In 1", "In 2", "Attack", "Release" };
        std::string defaultOutStrings[3] = { "Envelope" };

        GraphEffectEnvelopeFollower(NodeGraphBase* node) :
            NodeGraphOp(node, 4, 1, 0)
        {}

        virtual ~GraphEffectEnvelopeFollower() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual bool IsDataEditable(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Envelope Follower";
        }

    protected:
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectSaturator : public NodeGraphOp {
    public:
        std::string defaultInStrings[6] = { "In 1", "In 2", "Wet", "Preamp", "Limit", "Postamp"};
        std::string defaultOutStrings[3] = { "Out 1", "Out 2"};

        GraphEffectSaturator(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 2, 0)
        {}

        virtual ~GraphEffectSaturator() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual bool IsDataEditable(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Saturator";
        }

    protected:
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectTranceGate : public NodeGraphOp {
    public:
        std::string defaultInStrings[6] = { "In 1", "In 2", "Bpm", "Fmod", "Attack", "Pattern" };
        std::string defaultOutStrings[3] = { "Out 1", "Out 2" };

        const std::vector< std::vector<float>> patterns = {
            {0.7f, 0.0f, 0.7f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
            {0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
            {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
            {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f },
        };

        GraphEffectTranceGate(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 2, 0)
        {}

        virtual ~GraphEffectTranceGate() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual bool IsDataEditable(int8_t num) override {
            return num >= 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Trance Gate";
        }

    protected:
        float mSamplesUntilUpdate = 0.0f;
        int32_t mGateIndex = 0;
        std::vector<float> mGate;
        float mGainTarget = 1.0f;
        float mGain = 1.0f;
        float mGateSmoothing = 0.01f;
        float mGateSmoothingNeg = 0.01f;
    };


    /*********************************************************************/
    class GraphInputKeyboardPiano : public NodeGraphOp, public l::hid::INoteProcessor {
    public:
        GraphInputKeyboardPiano(NodeGraphBase* node, int32_t polyphony, l::hid::KeyState* keyState) :
            NodeGraphOp(node, 0, polyphony, polyphony)
        {
            mChannel.resize(polyphony);
            mKeyboard.SetKeyState(keyState);
            mKeyboard.SetNoteProcessor(this);
        }

        std::string defaultOutStrings[8] = { "Note 1", "Note 2", "Note 3", "Note 4", "Note 5", "Note 6", "Note 7", "Note 8" };

        virtual ~GraphInputKeyboardPiano() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tickCount, float elapsed) override;
        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Keyboard";
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        virtual void NoteOn(int32_t note) override;
        virtual void NoteOff() override;
        virtual void NoteOff(int32_t note) override;
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        int8_t mNoteCounter = 0;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
        l::hid::KeyboardPiano mKeyboard;
    };

    /*********************************************************************/
    class GraphInputMidiKeyboard : public NodeGraphOp {
    public:
        GraphInputMidiKeyboard(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, 0, 2, 2),
            mMidiManager(midiManager)
        {
            mChannel.resize(1);

            mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                MidiEvent(data);
                });
        }

        std::string defaultOutStrings[2] = { "Note", "Velocity"};

        virtual ~GraphInputMidiKeyboard() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Midi Device";
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
        void NoteOn(int32_t note, int32_t velocity);
        void NoteOff();
        void NoteOff(int32_t note);
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        l::hid::midi::MidiManager* mMidiManager = nullptr;

        int8_t mNoteCounter = 0;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
    };

    /*********************************************************************/
    class GraphInputMidiKnobs : public NodeGraphOp {
    public:
        GraphInputMidiKnobs(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, 0, 8, 8),
            mMidiManager(midiManager)
        {
            mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                MidiEvent(data);
                });
        }

        std::string defaultOutStrings[8] = { "Knob 1", "Knob 2", "Knob 3", "Knob 4", "Knob 5", "Knob 6", "Knob 7", "Knob 8", };

        virtual ~GraphInputMidiKnobs() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Midi Knobs";
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        l::hid::midi::MidiManager* mMidiManager = nullptr;
    };

    /*********************************************************************/

    enum class MidiButtonStates {
        DISCONNECTED = 0, // non lit
        ALLOCATED, // yellow blinking (in a node button group)
        CONNECTED, // yellow (output polled)
        OFF, // red, off - 0
        HALF_OFF, // red blinking - 0.33
        HALF_ON, // green blinking - 0.67
        ON, // green - 1
    };

    class GraphInputMidiButtons : public NodeGraphOp {
    public:

        // midi button commands
        // 0 - not lit
        // 1 - green
        // 2 - green flashing
        // 3 - red
        // 4 - red flashing
        // 5 - yellow
        // 6 - yellow flashing

        // remap to
        const int8_t BUTTON_DISCONNECTED = 0;  // non lit          - undefined
        const int8_t BUTTON_ALLOCATED = 1;     // yellow flashing  - undefined
        const int8_t BUTTON_OFF = 2;           // red              - 0.00f
        const int8_t BUTTON_ON_AND_OFF = 3;    // yellow           - 0.50f
        const int8_t BUTTON_ON = 4;            // green            - 1.00f

        const std::vector<int8_t> remapToButtonStatesToColor = {0, 6, 3, 5, 1, 4, 2};

        GraphInputMidiButtons(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager, int32_t buttonGroup) :
            NodeGraphOp(node, 0, 8, 8),
            mMidiManager(midiManager),
            mButtonGroup(buttonGroup)
        {
            mNodeName = "Midi Button Group ";
            mNodeName += std::to_string(mButtonGroup);
            mButtonStates.resize(8);

            for (int8_t i = 0; i < 8; i++) {
                defaultOutStrings[i] = std::string("Pad ") + std::to_string(i);
            }

            if (mMidiManager) {
                mCallbackId = mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                    MidiEvent(data);
                    });

                // turn all buttons on
                for (int8_t i = 0; i < 8; i++) {
                    UpdateButton(i, BUTTON_ALLOCATED);
                }
            }
        }

        std::string defaultOutStrings[8];

        virtual ~GraphInputMidiButtons() {
            // turn off all buttons before destructing
            for (int8_t i = 0; i < 8; i++) {
                UpdateButton(i, BUTTON_DISCONNECTED);
            }

            std::this_thread::yield();

            mMidiManager->UnregisterCallback(mCallbackId);
        }
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tick, float deltaTime) override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return mNodeName;
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        void UpdateButton(int8_t buttonId, int8_t buttonState) {
            if (mMidiManager) {
                int8_t buttonColor = remapToButtonStatesToColor.at(buttonState);
                mMidiManager->SendToDevice(0, 0x90, 0, mButtonGroup * 8 + buttonId, buttonColor);
                mButtonStates.at(buttonId) = buttonState;
            }
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mCallbackId = 0;
        int32_t mButtonGroup;
        std::string mNodeName;
        bool mMidiShiftState = false;
        std::vector<int8_t> mButtonStates;
        int32_t mInitCounter = 0;
    };
}

