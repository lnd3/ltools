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

    class GraphNumericAdd : public NodeGraphOp {
    public:
        GraphNumericAdd() :
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
        GraphNumericMultiply() : 
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
        GraphNumericSubtract() :
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
        GraphNumericNegate() :
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
        GraphNumericIntegral() :
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
        GraphLogicalAnd() :
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
        GraphLogicalOr() :
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
        GraphLogicalXor() :
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

        GraphFilterLowpass() :
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
