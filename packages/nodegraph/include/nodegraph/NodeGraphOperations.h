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
        void ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(float) override;

        std::string_view GetName() override;
        bool IsDataVisible(int8_t) override;
        bool IsDataEditable(int8_t) override;

    protected:
        int32_t mMode;
        float mMax = 1.0f;
        float mMin = 0.0f;
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
        float mPrevTime = 0.0f;
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
        void Tick(float time) override;
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

