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

    /* Logical operations */

    /*********************************************************************/
    class GraphLogicalAnd : public NodeGraphOp {
    public:
        GraphLogicalAnd(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalAnd() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            bool input1 = inputs.at(0).Get() != 0.0f;
            bool input2 = inputs.at(1).Get() != 0.0f;
            outputs.at(0).mOutput = (input1 && input2) ? 1.0f : 0.0f;
        }
        std::string_view GetName() override {
            return "And";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalOr() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            bool input1 = inputs.at(0).Get() != 0.0f;
            bool input2 = inputs.at(1).Get() != 0.0f;
            outputs.at(0).mOutput = (input1 || input2) ? 1.0f : 0.0f;
        }
        std::string_view GetName() override {
            return "Or";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

    /*********************************************************************/
    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(NodeGraphBase* node) :
            NodeGraphOp(node, 2, 1)
        {}

        virtual ~GraphLogicalXor() = default;
        virtual void Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            bool input1 = inputs.at(0).Get() != 0.0f;
            bool input2 = inputs.at(1).Get() != 0.0f;
            outputs.at(0).mOutput = (input1 ^ input2) ? 1.0f : 0.0f;
        }
        virtual std::string_view GetName() override {
            return "Xor";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    };

}

