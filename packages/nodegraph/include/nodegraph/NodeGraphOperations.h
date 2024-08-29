#pragma once
#include "nodegraph/NodeGraph.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>

namespace l::nodegraph {

    /* Mathematical operations */

    class GraphSourceConstants : public NodeGraphOp {
    public:
        GraphSourceConstants(int8_t mode = 0) :
            NodeGraphOp(0, 1, 1)
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
                mMax = 0.0f;
                mMin = 0.0f;
                break;
            }
        }

        virtual ~GraphSourceConstants() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Constant";
        }

    protected:
        float mMax = 1.0f;
        float mMin = 0.0f;
    };

    class GraphSourceSine : public NodeGraphOp {
    public:
        GraphSourceSine(int8_t) :
            NodeGraphOp(4, 2)
        {}

        std::string defaultInStrings[4] = { "Time", "Freq Hz", "Freq Mod", "Phase Mod"};
        std::string defaultOutStrings[2] = { "Sine", "Phase"};

        virtual ~GraphSourceSine() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

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

    class GraphNumericAdd : public NodeGraphOp {
    public:
        GraphNumericAdd(int8_t) :
            NodeGraphOp(2, 1)
        {}
        virtual ~GraphNumericAdd() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Add";
        }
    };

    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(int8_t) :
            NodeGraphOp(2, 1)
        {}

        virtual ~GraphNumericMultiply() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Multiply";
        }
    };

    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(int8_t) :
            NodeGraphOp(2, 1)
        {}
        virtual ~GraphNumericSubtract() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Subtract";
        }
    };

    class GraphNumericNegate : public NodeGraphOp {
    public:
        GraphNumericNegate(int8_t) :
            NodeGraphOp(1, 1)
        {}

        virtual ~GraphNumericNegate() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Negate";
        }
    };

    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(int8_t) :
            NodeGraphOp(1, 1)
        {}

        virtual ~GraphNumericIntegral() = default;
        void Reset() override;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Integral";
        }

    protected:
        float mOutput = 0.0f;
    };

    /* Logical operations */

    class GraphLogicalAnd : public NodeGraphOp {
    public:
        GraphLogicalAnd(int8_t) :
            NodeGraphOp(2, 1)
        {}

        virtual ~GraphLogicalAnd() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "And";
        }
    };

    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(int8_t) :
            NodeGraphOp(2, 1)
        {}

        virtual ~GraphLogicalOr() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Or";
        }
    };

    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(int8_t) :
            NodeGraphOp(2, 1)
        {}

        virtual ~GraphLogicalXor() = default;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        std::string_view GetName() override {
            return "Xor";
        }
    };

    /* Stateful filtering operations */

    class GraphFilterLowpass : public NodeGraphOp {
    public:
        std::string defaultInStrings[3] = { "Cutoff", "Resonance", "Data"};
        std::string defaultOutStrings[1] = { "Out" };

        GraphFilterLowpass(int8_t) :
            NodeGraphOp(3, 1)
        {}

        virtual ~GraphFilterLowpass() = default;
        void Reset() override;
        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

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


}

