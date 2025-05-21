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
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);
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
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);
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
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 ^ bool2) ? 1.0f : 0.0f;
            }
        }
    };

    /*********************************************************************/
    class GraphLogicalFlipGate : public NodeGraphOp {
    public:
        GraphLogicalFlipGate(NodeGraphBase* node) :
            NodeGraphOp(node, "Flip Gate")
        {
            AddInput("In", 0.0f);
            AddInput("Max input", 1.0f, 1, 0.00001f, l::math::constants::FLTMAX);
            AddOutput("Gate", 0.0f);
            AddOutput("Strength", 0.0f);
        }

        virtual ~GraphLogicalFlipGate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = &inputs.at(0).Get(numSamples);
            auto maxInput = &inputs.at(1).Get(numSamples);
            auto gate = &outputs.at(0).Get(numSamples);
            auto strength = &outputs.at(1).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                auto in = (*input++);
                bool pos = in > 0.01f;
                bool neg = in < -0.01f;
                
                if (mGate && neg) {
                    mGate = false;
                }
                if (!mGate && pos) {
                    mGate = true;
                }

                *gate++ = mGate ? 1.0f : -1.0f;
                *strength++ = in / (*maxInput);
            }
        }
    protected:
        bool mGate = false;
        float mStrength = 0.0f;
    };
}

