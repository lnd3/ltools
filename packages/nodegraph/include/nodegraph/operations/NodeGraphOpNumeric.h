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
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }
        virtual ~GraphNumericAdd() = default;
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
    class GraphNumericMultiply : public NodeGraphOp {
    public:
        GraphNumericMultiply(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphNumericMultiply() = default;
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
    class GraphNumericSubtract : public NodeGraphOp {
    public:
        GraphNumericSubtract(NodeGraphBase* node) :
            NodeGraphOp(node, "Subtract")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }
        virtual ~GraphNumericSubtract() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input1 = inputs.at(1).GetIterator(numSamples);
            auto lodExp = inputs.at(2).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

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
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphNumericNegate() = default;
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
    class GraphNumericIntegral : public NodeGraphOp {
    public:
        GraphNumericIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, "Integral")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Friction", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~GraphNumericIntegral() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto friction = inputs.at(1).Get();
            auto frictionFactor = l::math::pow(friction, 0.25f);
            auto lodExp = inputs.at(2).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                mOutput += *input0++;
                mOutput *= frictionFactor;
                *output++ = mOutput;
            }
        }
        virtual void Reset() override {
            mOutput = 0.0f;
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
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphNumericMultiply3() = default;
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
    class GraphNumericMultiplyAndAdd : public NodeGraphOp {
    public:
        GraphNumericMultiplyAndAdd(NodeGraphBase* node) :
            NodeGraphOp(node, "Multiply & Add")
        {
            AddInput("In 1");
            AddInput("In 2");
            AddInput("In 3");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphNumericMultiplyAndAdd() = default;
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
    class GraphNumericRound : public NodeGraphOp {
    public:
        GraphNumericRound(NodeGraphBase* node) :
            NodeGraphOp(node, "Round")
        {
            AddInput("In");
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphNumericRound() = default;
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

    /*********************************************************************/
    class GraphNumericDerivate : public NodeGraphOp {
    public:
        GraphNumericDerivate(NodeGraphBase* node) :
            NodeGraphOp(node, "Derivate")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~GraphNumericDerivate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto lodExp = inputs.at(1).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                float input = *input0++;
                float value = input - mInputPrev;
                float divisor = l::math::abs(input) + l::math::abs(mInputPrev);
                if (divisor > 0.0f) {
                    value = 2.0f * value / divisor;
                }
                mInputPrev = input;
                *output++ = value;
            }
        }
    protected:
        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class GraphNumericDiffNorm : public NodeGraphOp {
    public:
        GraphNumericDiffNorm(NodeGraphBase* node) :
            NodeGraphOp(node, "Difference Normalized")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~GraphNumericDiffNorm() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto lodExp = inputs.at(1).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                float input = *input0++;
                float value = mInputPrev;
                if (mInputPrev != 0.0f) {
                    if (input > 0.0f && mInputPrev > 0.0f) {
                        value = input / mInputPrev;
                        value = value - 1.0f;
                    }
                    else if (input < 0.0f && mInputPrev < 0.0f) {
                        value = input / mInputPrev;
                        value = (value - 1.0f);
                    }
                    else {
                        value = 0.0f;
                    }
                }
                mInputPrev = input;
                *output++ = value;
            }
        }
    protected:
        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class GraphNumericDiff : public NodeGraphOp {
    public:
        GraphNumericDiff(NodeGraphBase* node) :
            NodeGraphOp(node, "Difference")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~GraphNumericDiff() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto lodExp = inputs.at(1).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                float input = *input0++;
                float value = input - mInputPrev;
                mInputPrev = input;
                *output++ = value;
            }
        }
    protected:
        float mInputPrev = 0.0f;
    };

}