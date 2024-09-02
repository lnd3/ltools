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

namespace l::nodegraph {

    /* Mathematical operations */

    class GraphSourceConstants : public NodeGraphOp {
    public:
        GraphSourceConstants(NodeGraphBase* node, int32_t mode) :
            NodeGraphOp(node, 0, 4, 4),
            mMode(mode)
        {
            switch (mode) {
            case 0:
                mMax = 1.0f;
                mMin = 0.0f;
                break;
            case 1:
                mMax = 1.0f;
                mMin = -1.0f;
                break;
            case 2:
                mMax = 100.0f;
                mMin = 0.0f;
                break;
            default:
                mMax = 3.402823466e+38F;
                mMin = -3.402823466e+38F;
                break;
            }
        }

        virtual ~GraphSourceConstants() = default;
        virtual void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float, float) override;

        std::string_view GetName() override;
        bool IsDataVisible(int8_t) override;
        bool IsDataEditable(int8_t) override;

    protected:
        int32_t mMode;
        float mMax = 1.0f;
        float mMin = 0.0f;
    };

    class GraphSourceTime : public NodeGraphOp {
    public:
        GraphSourceTime(NodeGraphBase* node) :
            NodeGraphOp(node, 0, 2, 0)
        {}

        std::string defaultOutStrings[2] = { "Audio Time", "Frame Time"};

        virtual ~GraphSourceTime() = default;
        virtual void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float, float) override;

        void Reset() override;
        std::string_view GetOutputName(int8_t outputChannel);
        std::string_view GetName() override;

    protected:
        float mAudioTime = 0.0f;
        float mFrameTime = 0.0f;
    };

    class GraphSourceSine : public NodeGraphOp {
    public:
        GraphSourceSine(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 3)
        {}

        std::string defaultInStrings[5] = { "Time", "Freq Hz", "Freq Mod", "Phase Mod", "Reset"};
        std::string defaultOutStrings[3] = { "Sine", "Phase", "Phase Mod"};

        virtual ~GraphSourceSine() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        void Reset() override;
        std::string_view GetInputName(int8_t inputChannel);
        std::string_view GetOutputName(int8_t outputChannel);
        std::string_view GetName() override;

    protected:
        float mPhase = 0.0f;
    };

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
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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

    class GraphNumericAdd : public NodeGraphOp {
    public:
        GraphNumericAdd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericAdd() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Add";
        }
    };

    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphNumericMultiply() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Multiply";
        }
    };

    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericSubtract() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Subtract";
        }
    };

    class GraphNumericNegate : public NodeGraphOp {
    public:
        GraphNumericNegate(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericNegate() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Negate";
        }
    };

    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericIntegral() = default;
        void Reset() override;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Integral";
        }

    protected:
        float mOutput = 0.0f;
    };

    /* Logical operations */

    class GraphLogicalAnd : public NodeGraphOp {
    public:
        GraphLogicalAnd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalAnd() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "And";
        }
    };

    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalOr() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Or";
        }
    };

    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalXor() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Xor";
        }
    };

    /* Stateful filtering operations */

    class GraphFilterLowpass : public NodeGraphOp {
    public:
        std::string defaultInStrings[3] = { "Cutoff", "Resonance", "Data"};
        std::string defaultOutStrings[1] = { "Out" };

        GraphFilterLowpass(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        virtual ~GraphFilterLowpass() = default;
        void Reset() override;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }

        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }

        std::string_view GetName() override {
            return "Lowpass";
        }
    protected:
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };

    class GraphOutputDebug : public NodeGraphOp {
    public:
        GraphOutputDebug(NodeGraphBase* node, int32_t numValueDisplays) :
            NodeGraphOp(node, numValueDisplays, 0, 0)
        {}

        bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual ~GraphOutputDebug() = default;

        std::string_view GetName() override {
            return "Debug";
        }
    };

    class GraphOutputSpeaker : public NodeGraphOp {
    public:
        std::string defaultOutStrings[2] = { "Left", "Right"};

        GraphOutputSpeaker(NodeGraphBase* node, l::audio::AudioStream* stream = nullptr) :
            NodeGraphOp(node, 2, 0, 0),
            mAudioStream(stream),
            mCurrentStereoPosition(0)
        {}

        bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual ~GraphOutputSpeaker() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float time, float elapsed) override;

        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }

        std::string_view GetName() override {
            return "Speaker";
        }

    protected:
        l::audio::AudioStream* mAudioStream;
        int32_t mCurrentStereoPosition;
    };

    class GraphEffectReverb : public NodeGraphOp {
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

        GraphEffectReverb(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 2, 6)
        {
            //node->SetInput(2, 0.1f); // mix
            //node->SetInput(3, 0.7f); // attenuation
            //node->SetInput(4, 30.0f); // room size

            //node->SetInput(5, 0.2f); // delay 1
            //node->SetInput(6, 0.5f); // feedback 1

            //node->SetInput(7, 2.3f); // delay 2
            //node->SetInput(8, 0.79f); // feedback 2

            //node->SetInput(9, 0.5f); // delay 3
            //node->SetInput(10, 0.96f); // feedback 3

            uint32_t bufferSize = GetFramesPerRoomSize(maxRoomSizeInMeters);
            buf0.resize(bufferSize);
            buf1.resize(bufferSize);
        }

        virtual ~GraphEffectReverb() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

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
            return "Reverb";
        }
        float max(float value, float max) {
            return value > max ? value : max;
        }
        float min(float value, float min) {
            return value < min ? value : min;
        }

    protected:
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

}

