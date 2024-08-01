#pragma once
#include "GraphNode.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>

namespace l::graph {

    /* Mathematical operations */

    class GraphNumericAdd : public GraphOp {
    public:
        GraphNumericAdd() :
            GraphOp(2, 1)
        {}
        virtual ~GraphNumericAdd() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    class GraphNumericMultiply : public GraphOp {
    public:
        GraphNumericMultiply() : 
            GraphOp(2, 1)
        {}

        virtual ~GraphNumericMultiply() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    class GraphNumericSubtract : public GraphOp {
    public:
        GraphNumericSubtract() :
            GraphOp(2, 1)
        {}
        virtual ~GraphNumericSubtract() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    class GraphNumericNegate : public GraphOp {
    public:
        GraphNumericNegate() :
            GraphOp(1, 1)
        {}

        virtual ~GraphNumericNegate() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    class GraphNumericIntegral : public GraphOp {
    public:
        GraphNumericIntegral() :
            GraphOp(1, 1)
        {}

        virtual ~GraphNumericIntegral() = default;
        void Reset() override;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;

    protected:
        float mOutput = 0.0f;
    };

    /* Logical operations */

    class GraphLogicalAnd : public GraphOp {
    public:
        GraphLogicalAnd() :
            GraphOp(2, 1)
        {}

        virtual ~GraphLogicalAnd() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    class GraphLogicalOr : public GraphOp {
    public:
        GraphLogicalOr() :
            GraphOp(2, 1)
        {}

        virtual ~GraphLogicalOr() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    class GraphLogicalXor : public GraphOp {
    public:
        GraphLogicalXor() :
            GraphOp(2, 1)
        {}

        virtual ~GraphLogicalXor() = default;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    /* Stateful filtering operations */

    class GraphFilterLowpass : public GraphOp {
    public:
        GraphFilterLowpass() :
            GraphOp(3, 1)
        {}

        virtual ~GraphFilterLowpass() = default;
        void Reset() override;
        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;

    protected:
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };


}

