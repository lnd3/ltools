#pragma once
#include "nodegraph/NodeGraph.h"

#include "logging/LoggingAll.h"
#include "hid/KeyboardPiano.h"
#include "audio/PortAudio.h"

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
        virtual void Tick(float, float) override;

        virtual std::string_view GetName() override;
        virtual bool IsDataVisible(int8_t) override;
        virtual bool IsDataEditable(int8_t) override;

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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float, float) override;

        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual std::string_view GetName() override;

    protected:
        float mAudioTime = 0.0f;
        float mFrameTime = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceSine : public NodeGraphOp {
    public:
        GraphSourceSine(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 2)
        {}

        std::string defaultInStrings[5] = { "Note", "Volume", "Fmod", "Phase", "Reset"};
        std::string defaultOutStrings[2] = { "Sine", "Phase"};

        virtual ~GraphSourceSine() = default;

        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual std::string_view GetName() override;

    protected:
        float mPhase = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceKeyboard : public NodeGraphOp, public l::hid::INoteProcessor {
    public:
        GraphSourceKeyboard(NodeGraphBase* node, int32_t polyphony, l::hid::KeyState* keyState) :
            NodeGraphOp(node, 0, polyphony, polyphony)
        {
            mChannel.resize(polyphony);
            mKeyboard.SetKeyState(keyState);
            mKeyboard.SetNoteProcessor(this);
        }

        std::string defaultOutStrings[8] = { "Note 1", "Note 2", "Note 3", "Note 4", "Note 5", "Note 6", "Note 7", "Note 8" };

        virtual ~GraphSourceKeyboard() = default;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        void Tick(float time, float elapsed) override;
        void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override;
        virtual std::string_view GetName() override;
        virtual bool IsDataVisible(int8_t) override;
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
    class GraphNumericAdd : public NodeGraphOp {
    public:
        GraphNumericAdd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericAdd() = default;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Add";
        }
    };

    /*********************************************************************/
    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphNumericMultiply() = default;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Multiply";
        }
    };

    /*********************************************************************/
    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericSubtract() = default;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Subtract";
        }
    };

    /*********************************************************************/
    class GraphNumericNegate : public NodeGraphOp {
    public:
        GraphNumericNegate(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericNegate() = default;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Negate";
        }
    };

    /*********************************************************************/
    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericIntegral() = default;
        void Reset() override;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Integral";
        }

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
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Multiply3";
        }
    };

    /*********************************************************************/
    class GraphNumericMultiplyAndAdd : public NodeGraphOp {
    public:
        GraphNumericMultiplyAndAdd(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        std::string defaultInStrings[3] = { "Factor 1", "Factor 2", "Term 1" };

        virtual ~GraphNumericMultiplyAndAdd() = default;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Multiply & Add";
        }

        virtual std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
    };


    /* Logical operations */

    /*********************************************************************/
    class GraphLogicalAnd : public NodeGraphOp {
    public:
        GraphLogicalAnd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalAnd() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "And";
        }
    };

    /*********************************************************************/
    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalOr() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Or";
        }
    };

    /*********************************************************************/
    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalXor() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual std::string_view GetName() override {
            return "Xor";
        }
    };

    /* Stateful filtering operations */

    /*********************************************************************/
    class GraphFilterLowpass : public NodeGraphOp {
    public:
        std::string defaultInStrings[3] = { "Cutoff", "Resonance", "Data"};
        std::string defaultOutStrings[1] = { "Out" };

        GraphFilterLowpass(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        virtual ~GraphFilterLowpass() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        virtual std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }

        virtual std::string_view GetOutputName(int8_t outputChannel) {
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
        std::string defaultInStrings[5] = { "Note", "Attack", "Release", "Fade"};
        std::string defaultOutStrings[2] = { "Note", "Volume"};

        GraphFilterEnvelope(NodeGraphBase* node) :
            NodeGraphOp(node, 4, 2)
        {}

        virtual ~GraphFilterEnvelope() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float, float) {}

        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }

        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }

        bool IsDataVisible(int8_t channel) {
            return channel >= 1 ? true : false;
        }

        bool IsDataEditable(int8_t channel) {
            return channel >= 1 ? true : false;
        }

        std::string_view GetName() override {
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
        virtual std::string_view GetOutputName(int8_t outputChannel) {
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

        void Reset() override;
        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        bool IsDataVisible(int8_t) override {
            return true;
        }
        bool IsDataEditable(int8_t channel) override {
            return channel == 2 ? true : false;
        }
        std::string_view GetInputName(int8_t outputChannel) {
            return defaultInStrings[outputChannel];
        }

        std::string_view GetName() override {
            return "Speaker";
        }

    protected:
        l::audio::AudioStream* mAudioStream;
        int32_t mCurrentStereoPosition;
        float mEnvelope = 0.0f;
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
        
        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }

        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }
        
        std::string_view GetName() override {
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

        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }

        std::string_view GetName() override {
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
        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }
        std::string_view GetName() override {
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
        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }
        std::string_view GetName() override {
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
        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }
        std::string_view GetName() override {
            return "Saturator";
        }

    protected:
        float mEnvelope = 0.0f;
    };
}

