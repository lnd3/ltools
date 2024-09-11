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
    class GraphEffectEnvelope : public NodeGraphOp {
    public:
        std::string defaultInStrings[5] = { "Freq", "Velocity", "Attack", "Release", "Fade" };
        std::string defaultOutStrings[2] = { "Freq", "Volume" };

        GraphEffectEnvelope(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 2)
        {}

        virtual ~GraphEffectEnvelope() = default;
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
        std::string defaultInStrings[7] = { "In 1", "In 2", "Bpm", "Fmod", "Attack", "Pattern", "Drift Sync"};
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
            NodeGraphOp(node, 7, 2, 0)
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
        float mGainTarget = 1.0f;
        float mGain = 1.0f;
        float mGateSmoothing = 0.01f;
        float mGateSmoothingNeg = 0.01f;
    };

    /*********************************************************************/
    class GraphEffectArpeggio: public NodeGraphOp {
    public:
        std::string defaultInStrings[7] = { "Note On Id", "Note Off Id", "Velocity", "Bpm", "Fmod", "Attack", "Drift Sync"};
        std::string defaultOutStrings[3] = { "Freq", "Volume" };

        GraphEffectArpeggio(NodeGraphBase* node) :
            NodeGraphOp(node, 7, 2, 0)
        {}

        virtual ~GraphEffectArpeggio() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        virtual bool IsDataEditable(int8_t channel) override {
            return channel > 2 ? true : false;
        }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Arpeggio";
        }

    protected:
        float mSamplesUntilUpdate = 0.0f;
        float mGainTarget = 0.0f;
        float mGain = 0.0f;
        float mGainSmoothing = 0.01f;
        float mGainSmoothingNeg = 0.01f;

        float mCurrentNoteFreq = 0.0f;
        std::vector<int32_t> mNotes;
        int32_t mNoteIndex = 0;
    };
}

