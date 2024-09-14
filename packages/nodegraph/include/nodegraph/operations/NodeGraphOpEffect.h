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

    class GraphEffectBase : public NodeGraphOp {
    public:

        static const int8_t mNumDefaultInputs = 6;
        static const int8_t mNumDefaultOutputs = 2;

        GraphEffectBase(NodeGraphBase* node, std::string_view name) :
            NodeGraphOp(node, name)
        {
            AddInput("Sync", 0.0f, 1, 0.0f, 1.0f);
            AddInput("Rate", 256.0f, 1, 1.0f, 2048.0f);
            AddInput("Gain", 0.5f, 1, 0.0f, 5.0f);
            AddInput("Mix", 0.5f, 1, 0.0f, 1.0f);
            AddInput("In 0");
            AddInput("In 1");

            AddOutput("Out 1");
            AddOutput("Out 2");

            mDeltaTime = 0.0f;
            mSamplesUntilUpdate = 0.0f;
        }

        virtual ~GraphEffectBase() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override final;
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual std::pair<float, float> ProcessStereoSignal(float deltaTime, float value0, float value1) = 0;
    protected:
        float mUpdateRate = 256.0f;

        float mDeltaTime = 0.0f;
        float mSamplesUntilUpdate = 0.0f;

        l::audio::FilterRWA<float> mFilterGain;
        l::audio::FilterRWA<float> mFilterMix;
    };

    /*********************************************************************/
    class GraphEffectReverb2 : public GraphEffectBase {
    public:
        uint32_t GetFramesPerRoomSize(float roomSize) {
            const float metersPerFrame = 334.0f / 44100.0f; // (m/s)/(frames/s) = m/frames;
            const float metersToWallPerFrame = metersPerFrame / 2.0f; // half the distance to wall for the bounced distance
            const float framesPerRoom = roomSize / metersToWallPerFrame;
            return static_cast<uint32_t>(framesPerRoom);
        }

        const float maxRoomSizeInMeters = 334.0f; // 334 meters large is 2 seconds of reverbation

        GraphEffectReverb2(NodeGraphBase* node) :
            GraphEffectBase(node, "Reverb 3") {

            uint32_t bufferSize = GetFramesPerRoomSize(maxRoomSizeInMeters);
            mBuf0.resize(bufferSize);
            mBuf1.resize(bufferSize);

            AddInput("Attenuation", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Room Size", 30.0f, 1, 0.2f, maxRoomSizeInMeters);
            AddInput("Delay 1", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Feedback 1", 0.9f, 1, 0.0f, 1.0f);
            AddInput("Delay 2", 0.8f, 1, 0.0f, 1.0f);
            AddInput("Feedback 2", 0.9f, 1, 0.0f, 1.0f);
            AddInput("Delay 3", 0.7f, 1, 0.0f, 1.0f);
            AddInput("Feedback 3", 0.9f, 1, 0.0f, 1.0f);
        }

        virtual ~GraphEffectReverb2() = default;
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) override;
        virtual std::pair<float, float> ProcessStereoSignal(float deltaTime, float value0, float value1) override;
    protected:
        std::vector<float> mBuf0;
        std::vector<float> mBuf1;
        uint32_t mBufIndex = 0;
        uint32_t mBufSizeLimit = 1;
        uint32_t mDelay0 = 0;
        uint32_t mDelay1 = 0;
        uint32_t mDelay2 = 0;

        float fb = 0.0f;
        float fb0 = 0.0f;
        float fb1 = 0.0f;
        float fb2 = 0.0f;
        float d0 = 0.0f;
        float d1 = 0.0f;
        float d2 = 0.0f;
    };
    /*********************************************************************/

    class GraphEffectReverb1 : public NodeGraphOp {
    public:
        uint32_t GetFramesPerRoomSize(float roomSize) {
            const float metersPerFrame = 334.0f / 44100.0f; // (m/s)/(frames/s) = m/frames;
            const float metersToWallPerFrame = metersPerFrame / 2.0f; // half the distance to wall for the bounced distance
            const float framesPerRoom = roomSize / metersToWallPerFrame;
            return static_cast<uint32_t>(framesPerRoom);
        }

        const float maxRoomSizeInMeters = 334.0f; // 334 meters large is 2 seconds of reverbation

        GraphEffectReverb1(NodeGraphBase * node) :
            NodeGraphOp(node, "Reverb 1") {

            uint32_t bufferSize = GetFramesPerRoomSize(maxRoomSizeInMeters);
            buf0.resize(bufferSize);
            buf1.resize(bufferSize);

            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddInput("Mix", 0.75f, 1, 0.0f, 1.0f);
            AddInput("Attenuation", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Room Size", 30.0f, 1, 0.2f, maxRoomSizeInMeters);
            AddInput("Delay 1", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Feedback 1", 0.9f, 1, 0.0f, 1.0f);
            AddInput("Delay 2", 0.8f, 1, 0.0f, 1.0f);
            AddInput("Feedback 2", 0.9f, 1, 0.0f, 1.0f);
            AddInput("Delay 3", 0.7f, 1, 0.0f, 1.0f);
            AddInput("Feedback 3", 0.9f, 1, 0.0f, 1.0f);

            AddOutput("Out 1");
            AddOutput("Out 2");
        }
            
        virtual ~GraphEffectReverb1() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>&inputs, std::vector<NodeGraphOutput>&outputs) override;
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
    class GraphEffectReverb3 : public NodeGraphOp {
    public:
        uint32_t GetFramesPerRoomSize(float roomSize) {
            const float metersPerFrame = 334.0f / 44100.0f; // (m/s)/(frames/s) = m/frames;
            const float metersToWallPerFrame = metersPerFrame / 2.0f; // half the distance to wall for the bounced distance
            const float framesPerRoom = l::math::functions::min(5.0f, roomSize / metersToWallPerFrame);
            return static_cast<uint32_t>(framesPerRoom);
        }

        const float maxRoomSizeInMeters = 334.0f; // 334 meters large is 2 seconds of reverbation

        GraphEffectReverb3(NodeGraphBase* node) :
            NodeGraphOp(node, "Reverb 2")
        {
            uint32_t bufferSize = GetFramesPerRoomSize(maxRoomSizeInMeters);
            bufRev0.resize(bufferSize);
            bufRev1.resize(bufferSize);
            bufEarlyTap0.resize(bufferSize);
            bufEarlyTap1.resize(bufferSize);

            AddInput("In 1");
            AddInput("In 2");
            AddInput("Mix", 0.3f, 1, 0.0f, 1.0f);
            AddInput("Feedback", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Room Size", 30.0f, 1, 0.1f, maxRoomSizeInMeters);
            AddInput("Width", 0.5f, 1, 0.0f, 1.0f);
            AddInput("First tap", 0.1f, 1, 0.0f, 10.0f);
            AddInput("Longest tap", 0.8f, 1, 0.0f, 10.0f);
            AddInput("Num tapss", 5.0f, 1, 1.0f, 30.0f);
            AddInput("Tap bulge", 0.7f, 1, 1.0f, 10.0f);
            AddInput("Filter cutoff", 0.95f, 1, 0.001f, 0.999f);
            AddInput("Filter res", 0.01f, 1, 0.001f, 0.999f);

            AddOutput("Rev 1");
            AddOutput("Rev 2");
            AddOutput("Tap 1");
            AddOutput("Tap 2");
        }

        virtual ~GraphEffectReverb3() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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
        GraphEffectLimiter(NodeGraphBase* node) :
            NodeGraphOp(node, "Limiter")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Attack", 5.0f, 1, 1.0f, 10000.0f);
            AddInput("Release", 100.0f, 1, 1.0f, 10000.0f);
            AddInput("Preamp", 1.0f, 1, 0.0f, 10.0f);
            AddInput("Limit", 0.95f, 1, 0.0f, 10.0f);

            AddOutput("Out 1");
            AddOutput("Out 2");
            AddOutput("Envelope");

            mEnvelope = 0.0f;
        }

        virtual ~GraphEffectLimiter() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectEnvelope : public NodeGraphOp {
    public:
        GraphEffectEnvelope(NodeGraphBase* node) :
            NodeGraphOp(node, "Envelope")
        {
            AddInput("Freq");
            AddInput("Velocity", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Attack", 50.0f, 1, 1.0f, 10000.0f);
            AddInput("Release", 50.0f, 1, 1.0f, 10000.0f);
            AddInput("Fade", 0.1f, 1, 0.0001f, 1.0f);

            AddOutput("Freq");
            AddOutput("Volume");

            mEnvelope = 0.0f;
        }

        virtual ~GraphEffectEnvelope() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) {}
    protected:
        int32_t mFrameCount = 0;
        float mEnvelopeTarget = 0.0f;
        float mFreq = 0.0f;
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectEnvelopeFollower : public NodeGraphOp {
    public:
        GraphEffectEnvelopeFollower(NodeGraphBase* node) :
            NodeGraphOp(node, "Envelope Follower")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Attack", 5.0f, 1, 1.0f, 10000.0f);
            AddInput("Release", 100.0f, 1, 1.0f, 10000.0f);

            AddOutput("Envelope");

            mEnvelope = 0.0f;
        }

        virtual ~GraphEffectEnvelopeFollower() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectSaturator : public NodeGraphOp {
    public:
        GraphEffectSaturator(NodeGraphBase* node) :
            NodeGraphOp(node, "Saturator")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Wet", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Preamp", 1.5f, 1, 0.0f, 10.0f);
            AddInput("Limit", 0.6f, 1, 0.0f, 10.0f);
            AddInput("Postamp", 1.4f, 1, 0.0f, 10.0f);

            AddOutput("Out 1");
            AddOutput("Out 2");

            mEnvelope = 0.0f;
        }

        virtual ~GraphEffectSaturator() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphEffectTranceGate : public NodeGraphOp {
    public:
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
            NodeGraphOp(node, "Trance Gate")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Bpm", 60.0f, 1, 1.0f, 1000.0f);
            AddInput("Fmod", 1.0f, 1, 0.01f, 1.0f);
            AddInput("Attack", 0.001f, 1, 0.0f, 1.0f);
            AddInput("Pattern", 0.0f, 1, 0.0f, 100.0f);
            AddInput("Sync", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("Out 1");
            AddOutput("Out 2");

            mGateIndex = 0;
        }

        virtual ~GraphEffectTranceGate() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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

        const static int32_t gPolyphony = 12;
        GraphEffectArpeggio(NodeGraphBase* node) :
            NodeGraphOp(node, "Arpeggio")
        {
            AddInput("Note On", l::audio::gNoNote_f, gPolyphony, -499.0, 500.0);
            AddInput("Note Off", l::audio::gNoNote_f, gPolyphony, -499.0, 500.0);
            AddInput("Velocity", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Bpm", 60.0f, 1, 1.0f, 1000.0f);
            AddInput("Fmod", 1.0f, 1, 0.01f, 1.0f);
            AddInput("Attack", 0.01f, 1, 0.0f, 1.0f);
            AddInput("Smooth", 0.0f, 1, 0.0f, 1.0f);
            AddInput("Sync", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("Freq");
            AddOutput("Volume");

            mGainTarget = 0.0f;
        }

        virtual ~GraphEffectArpeggio() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        float mSamplesUntilUpdate = 0.0f;
        float mGainTarget = 0.0f;
        float mGain = 0.0f;
        float mGainSmoothing = 0.01f;
        float mGainSmoothingNeg = 0.01f;

        float mFreqSmoothing = 0.1f;
        float mFreqTarget = 0.0f;
        float mFreq = 0.0f;
        std::vector<int32_t> mNotes;
        int32_t mNoteIndex = 0;
    };
}

