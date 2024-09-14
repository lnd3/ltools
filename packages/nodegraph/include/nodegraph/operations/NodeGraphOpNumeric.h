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
    class GraphNumericAdd : public NodeGraphOp {
    public:
        GraphNumericAdd(NodeGraphBase* node) :
            NodeGraphOp(node, "Add")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddOutput("Out");
        }
        virtual ~GraphNumericAdd() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() + inputs.at(1).Get();
        }
    };

    /*********************************************************************/
    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddOutput("Out");
        }

        virtual ~GraphNumericMultiply() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get();
        }
    };

    /*********************************************************************/
    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, "Subtract")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddOutput("Out");
        }
        virtual ~GraphNumericSubtract() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() - inputs.at(1).Get();
        }
    };

    /*********************************************************************/
    class GraphNumericNegate : public NodeGraphOp {
    public:
        GraphNumericNegate(NodeGraphBase* node) :
            NodeGraphOp(node, "Negate")
        {
            AddInput("In");
            AddOutput("Out");
        }

        virtual ~GraphNumericNegate() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = -inputs.at(0).Get();
        }
    };

    /*********************************************************************/
    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, "Integral")
        {
            AddInput("In");
            AddOutput("Out");
        }

        virtual ~GraphNumericIntegral() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            mOutput += inputs.at(0).Get();
            outputs.at(0).mOutput = mOutput;
        }
    protected:
        float mOutput = 0.0f;
    };

    /*********************************************************************/
    class GraphNumericMultiply3 : public NodeGraphOp {
    public:
        GraphNumericMultiply3(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply3")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("In 3");
            AddOutput("Out");
        }

        virtual ~GraphNumericMultiply3() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get() * inputs.at(2).Get();
        }
    };

    /*********************************************************************/
    class GraphNumericMultiplyAndAdd : public NodeGraphOp {
    public:
        GraphNumericMultiplyAndAdd(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply & Add")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("In 3");
            AddOutput("Out");
        }

        virtual ~GraphNumericMultiplyAndAdd() = default;
        void virtual Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get() + inputs.at(2).Get();
        }
    };

    /*********************************************************************/
    class GraphNumericRound : public NodeGraphOp {
    public:
        GraphNumericRound(NodeGraphBase* node) :
            NodeGraphOp(node, "Round")
        {
            AddInput("In");
            AddOutput("Out");
        }
        virtual ~GraphNumericRound() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = l::math::functions::round(inputs.at(0).Get());
        }
    };

}

