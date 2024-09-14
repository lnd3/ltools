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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ + *input1++;
            }
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ * *input1++;
            }
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ - *input1++;
            }
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = - *input0++;
            }
        }
    };

    /*********************************************************************/
    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, "Integral")
        {
            AddInput("In", 0.0f, 1);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~GraphNumericIntegral() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                mOutput += *input0++;
                *output++ = mOutput;
            }
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto input2 = inputs.at(2).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ * *input1++ * *input2++;
            }
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
        void virtual Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto input2 = inputs.at(2).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ * *input1++ + *input2++;
            }
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = l::math::functions::round(inputs.at(0).Get());

            auto input0 = inputs.at(2).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            if (numSamples > 1) {
                ASSERT(numSamples == outputs.at(0).GetSize());
            }

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = l::math::functions::round(*input0++);
            }
        }
    };

}

