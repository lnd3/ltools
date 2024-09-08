#pragma once
#include "nodegraph/NodeGraph.h"

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
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericAdd() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() + inputs.at(1).Get();
        }
        virtual std::string_view GetName() override {
            return "Add";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphNumericMultiply() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get();
        }
        virtual std::string_view GetName() override {
            return "Multiply";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}
        virtual ~GraphNumericSubtract() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() - inputs.at(1).Get();
        }
        virtual std::string_view GetName() override {
            return "Subtract";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericNegate : public NodeGraphOp {
    public:
        GraphNumericNegate(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericNegate() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = -inputs.at(0).Get();
        }
        virtual std::string_view GetName() override {
            return "Negate";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}

        virtual ~GraphNumericIntegral() = default;
        virtual void Reset() override {
            mOutput = 0.0f;
        }

        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            mOutput += inputs.at(0).Get();
            outputs.at(0).mOutput = mOutput;
        }
        virtual std::string_view GetName() override {
            return "Integral";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
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
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get() * inputs.at(2).Get();
        }
        virtual std::string_view GetName() override {
            return "Multiply3";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericMultiplyAndAdd : public NodeGraphOp {
    public:
        GraphNumericMultiplyAndAdd(NodeGraphBase* node) :
            NodeGraphOp(node, 3, 1)
        {}

        std::string defaultInStrings[3] = { "Factor 1", "Factor 2", "Term 1" };

        virtual ~GraphNumericMultiplyAndAdd() = default;
        void virtual Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get() + inputs.at(2).Get();
        }
        virtual std::string_view GetName() override {
            return "Multiply & Add";
        }

        virtual std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphNumericRound : public NodeGraphOp {
    public:
        GraphNumericRound(NodeGraphBase* node) :
            NodeGraphOp(node, 1, 1)
        {}
        virtual ~GraphNumericRound() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = l::math::functions::round(inputs.at(0).Get());
        }
        virtual std::string_view GetName() override {
            return "Round";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

}

