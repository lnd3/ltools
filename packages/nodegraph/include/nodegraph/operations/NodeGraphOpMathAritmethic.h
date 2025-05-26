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
    class MathAritmethicAdd : public NodeGraphOp {
    public:
        MathAritmethicAdd(NodeGraphBase* node) :
            NodeGraphOp(node, "Add")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }
        virtual ~MathAritmethicAdd() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto lodExp = inputs.at(2).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ + *input1++;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicMultiply : public NodeGraphOp {
    public:
        MathAritmethicMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~MathAritmethicMultiply() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto lodExp = inputs.at(2).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ * *input1++;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicSubtract : public NodeGraphOp {
    public:
        MathAritmethicSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, "Subtract")
        {
            AddInput("In1");
            AddInput("In2");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
            AddOutput("In1-In2");
            AddOutput("In2-In1");
        }
        virtual ~MathAritmethicSubtract() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto lodExp = inputs.at(2).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output1 = outputs.at(0).GetIterator(numSamples, lodFactor);
            auto output2 = outputs.at(1).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                auto diff = *input0++ - *input1++;
                *output1++ = diff;
                *output2++ = -diff;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicNegate : public NodeGraphOp {
    public:
        MathAritmethicNegate(NodeGraphBase* node) :
            NodeGraphOp(node, "Negate")
        {
            AddInput("In");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~MathAritmethicNegate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto lodExp = inputs.at(1).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = -*input0++;
            }
        }
    };


    /*********************************************************************/
    class MathAritmethicAbs : public NodeGraphOp {
    public:
        MathAritmethicAbs(NodeGraphBase* node) :
            NodeGraphOp(node, "Abs")
        {
            AddInput("In");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("abs(In)");
            AddOutput("max(In,0)");
            AddOutput("min(In,0)");
        }

        virtual ~MathAritmethicAbs() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto lodExp = inputs.at(1).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output1 = outputs.at(0).GetIterator(numSamples, lodFactor);
            auto output2 = outputs.at(1).GetIterator(numSamples, lodFactor);
            auto output3 = outputs.at(2).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output1++ = l::math::abs(*input0++);
                *output2++ = l::math::max2(*input0++, 0.0f);
                *output3++ = l::math::min2(*input0++, 0.0f);
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicMultiply3 : public NodeGraphOp {
    public:
        MathAritmethicMultiply3(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply3")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("In 3");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~MathAritmethicMultiply3() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto input2 = inputs.at(2).GetIterator(numSamples);
            auto lodExp = inputs.at(3).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ * *input1++ * *input2++;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicMultiplyAndAdd : public NodeGraphOp {
    public:
        MathAritmethicMultiplyAndAdd(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply & Add")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("In 3");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~MathAritmethicMultiplyAndAdd() = default;
        void virtual Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto input2 = inputs.at(2).GetIterator(numSamples);
            auto lodExp = inputs.at(3).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ * *input1++ + *input2++;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicRound : public NodeGraphOp {
    public:
        MathAritmethicRound(NodeGraphBase* node) :
            NodeGraphOp(node, "Round")
        {
            AddInput("In");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~MathAritmethicRound() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = l::math::round(inputs.at(0).Get());

            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto lodExp = inputs.at(1).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = l::math::round(*input0++);
            }
        }
    };

}