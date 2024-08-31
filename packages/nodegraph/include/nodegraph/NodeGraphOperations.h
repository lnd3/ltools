#pragma once
#include "nodegraph/NodeGraph.h"

#include "logging/LoggingAll.h"
#include "hid/KeyboardPiano.h"

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
            NodeGraphOp(node, 0, 1, 1),
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
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float) override;

        std::string_view GetName() override {
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

        bool IsDataVisible(int8_t) override {
            return true;
        }

        bool IsDataEditable(int8_t) override {
            return true;
        }

    protected:
        int32_t mMode;
        float mMax = 1.0f;
        float mMin = 0.0f;
    };

    class GraphSourceSine : public NodeGraphOp {
    public:
        GraphSourceSine(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 2)
        {}

        std::string defaultInStrings[5] = { "Time", "Freq Hz", "Freq Mod", "Phase Mod", "Reset"};
        std::string defaultOutStrings[2] = { "Sine", "Phase"};

        virtual ~GraphSourceSine() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        void Reset() override {
            mPhase = 0.0f;
            mPrevTime = 0.0f;
        }

        std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }

        std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }

        std::string_view GetName() override {
            return "Sine";
        }

    protected:
        float mPhase = 0.0f;
        float mPrevTime = 0.0f;
    };

    namespace {
        float GetFrequencyFromNote(float note) {
            return 440.0f * powf(2.0f, (note - 49.0f) / 12.0f);
        }
    }

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
        void Tick(float time) override;

        void Reset() override {
            for (int8_t i = 0; i < GetNumInputs(); i++) {
                mNode->SetInput(i, 0.0f);
            }
            mNode->ProcessSubGraph();
        }

        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }

        virtual std::string_view GetName() override {
            return "Keyboard";
        }

        virtual bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual void NoteOn(int32_t note) override {
            float frequency = GetFrequencyFromNote(static_cast<float>(note));
            int8_t channel = GetNextNoteChannel(note);
            mNode->SetInput(static_cast<int8_t>(channel), frequency);
            mNode->ProcessSubGraph();
        }
        virtual void NoteOff() override {
            Reset();
        }

        virtual void NoteOff(int32_t note) override {
            int8_t channel = ResetNoteChannel(note);
            if (channel >= 0) {
                mNode->SetInput(channel, 0.0f);
                mNode->ProcessSubGraph();
            }
        }
    protected:
        int8_t ResetNoteChannel(int32_t note) {
            for (size_t i = 0; i < mChannel.size(); i++) {
                if (mChannel.at(i).first == note) {
                    mChannel.at(i).second = 0;
                    return static_cast<int8_t>(i);
                }
            }
            // It is possible to get a note off for a note not playing because the channel was taken for another newer note
            return -1;
        }

        int8_t GetNextNoteChannel(int32_t note) {
            for (size_t i = 0; i < mChannel.size(); i++) {
                if (mChannel.at(i).first == note) {
                    mChannel.at(i).second = mNoteCounter++;
                    return static_cast<int8_t>(i);
                }
            }

            for (size_t i = 0; i < mChannel.size(); i++) {
                if (mChannel.at(i).first == 0) {
                    mChannel.at(i).first = note;
                    mChannel.at(i).second = mNoteCounter++;
                    return static_cast<int8_t>(i);
                }
            }

            int32_t lowestCount = INT32_MAX;
            int8_t lowestCountIndex = 0;
            for (size_t i = 0; i < mChannel.size(); i++) {
                if (lowestCount > mChannel.at(i).second) {
                    lowestCount = mChannel.at(i).second;
                    lowestCountIndex = static_cast<int8_t>(i);
                }
            }
            mChannel.at(lowestCountIndex).first = note;
            mChannel.at(lowestCountIndex).second = mNoteCounter++;
            return lowestCountIndex;
        }

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

    class GraphGraphicDisplay : public NodeGraphOp {
    public:
        GraphGraphicDisplay(NodeGraphBase* node, int32_t numValueDisplays) :
            NodeGraphOp(node, numValueDisplays, 0, 0)
        {}

        bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual ~GraphGraphicDisplay() = default;
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float time) override;

        std::string_view GetName() override {
            return "Display";
        }
    };
}

