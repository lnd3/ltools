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
            NodeGraphOp(node, "And")
        {
            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddOutput("Out", 0.0f);
        }

        virtual ~GraphLogicalAnd() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator();
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 && bool2) ? 1.0f : 0.0f;
            }
        }
    };

    /*********************************************************************/
    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(NodeGraphBase* node) :
            NodeGraphOp(node, "Or")
        {
            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddOutput("Out", 0.0f);
        }

        virtual ~GraphLogicalOr() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator();
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 || bool2) ? 1.0f : 0.0f;
            }
        }
    };

    /*********************************************************************/
    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(NodeGraphBase* node) :
            NodeGraphOp(node, "Xor")
        {
            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddOutput("Out", 0.0f);
        }

        virtual ~GraphLogicalXor() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator();
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 ^ bool2) ? 1.0f : 0.0f;
            }
        }
    };

}

