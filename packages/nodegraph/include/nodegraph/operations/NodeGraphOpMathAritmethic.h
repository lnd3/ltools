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
            AddInput("a");
            AddInput("b");
            AddOutput("a+b");
        }
        virtual ~MathAritmethicAdd() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto input1 = &inputs.at(1).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

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
            AddInput("a");
            AddInput("b");
            AddOutput("a*b");
        }

        virtual ~MathAritmethicMultiply() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto input1 = &inputs.at(1).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

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
            AddInput("a");
            AddInput("b");
            AddOutput("a-b");
            AddOutput("b-a");
        }
        virtual ~MathAritmethicSubtract() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto input1 = &inputs.at(1).Get(numSamples);
            auto output1 = &outputs.at(0).Get(numSamples);
            auto output2 = &outputs.at(1).Get(numSamples);

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
            AddInput("x");
            AddOutput("-x");
        }

        virtual ~MathAritmethicNegate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

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
            AddInput("x");
            AddOutput("abs(x)");
            AddOutput("max(x,0)");
            AddOutput("min(x,0)");
        }

        virtual ~MathAritmethicAbs() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto output1 = &outputs.at(0).Get(numSamples);
            auto output2 = &outputs.at(1).Get(numSamples);
            auto output3 = &outputs.at(2).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                auto in = *input0++;
                *output1++ = l::math::abs(in);
                *output2++ = l::math::max2(in, 0.0f);
                *output3++ = l::math::min2(in, 0.0f);
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicLog : public NodeGraphOp {
    public:
        MathAritmethicLog(NodeGraphBase* node) :
            NodeGraphOp(node, "Log")
        {
            AddInput("x");
            AddInput("b", 2.72f, 1, 1.0f, 10.0f);
            AddOutput("Log");
            AddOutput("Logb");
        }

        virtual ~MathAritmethicLog() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto base = inputs.at(1).Get();
            auto output1 = &outputs.at(0).Get(numSamples);
            auto output2 = &outputs.at(1).Get(numSamples);

            // we want logb(in) = ln(in)/ln(b)
            // so precalc base factor 1/ln(b)
            auto logBase = 1.0f / l::math::log(base);

            for (int32_t i = 0; i < numSamples; i++) {
                auto in = *input0++;
                auto ln = l::math::log(in);
                *output1++ = ln;
                *output2++ = ln * logBase;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicMultiply3 : public NodeGraphOp {
    public:
        MathAritmethicMultiply3(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply3")
        {
            AddInput("a");
            AddInput("b");
            AddInput("c");
            AddOutput("abc");
        }

        virtual ~MathAritmethicMultiply3() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto input1 = &inputs.at(1).Get(numSamples);
            auto input2 = &inputs.at(2).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

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
            AddInput("a");
            AddInput("b");
            AddInput("c");
            AddOutput("ab+c");
        }

        virtual ~MathAritmethicMultiplyAndAdd() = default;
        void virtual Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto input1 = &inputs.at(1).Get(numSamples);
            auto input2 = &inputs.at(2).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

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
            AddInput("x");
            AddOutput("Out");
        }

        virtual ~MathAritmethicRound() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            outputs.at(0).mOutput = l::math::round(inputs.at(0).Get());

            auto input0 = &inputs.at(0).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = l::math::round(*input0++);
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicPow : public NodeGraphOp {
    public:
        MathAritmethicPow(NodeGraphBase* node) :
            NodeGraphOp(node, "Pow")
        {
            AddInput("x");
            AddInput("y", 2.72f, 1, 1.0f, 10.0f);
            AddOutput("x^y");
        }

        virtual ~MathAritmethicPow() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto exponent = inputs.at(2).Get();
            auto output1 = &outputs.at(0).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                auto in = *input0++;
                auto pow = l::math::pow(in, exponent);
                *output1++ = pow;
            }
        }
    };

    /*********************************************************************/
    class MathAritmethicSum3 : public NodeGraphOp {
    public:
        MathAritmethicSum3(NodeGraphBase* node) :
            NodeGraphOp(node, "Sum3")
        {
            AddInput("a");
            AddInput("b");
            AddInput("c");
            AddOutput("a+b+c");
        }
        virtual ~MathAritmethicSum3() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = &inputs.at(0).Get(numSamples);
            auto input1 = &inputs.at(1).Get(numSamples);
            auto input2 = &inputs.at(2).Get(numSamples);
            auto output = &outputs.at(0).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                *output++ = *input0++ + *input1++ + *input2++;
            }
        }
    };
}