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
    class MathNumericalIntegral : public NodeGraphOp {
    public:
        MathNumericalIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, "Integral")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Friction", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalIntegral() = default;
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
    class MathNumericalDerivate : public NodeGraphOp {
    public:
        MathNumericalDerivate(NodeGraphBase* node) :
            NodeGraphOp(node, "Derivate")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalDerivate() = default;
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
    class MathNumericalDiffNorm : public NodeGraphOp {
    public:
        MathNumericalDiffNorm(NodeGraphBase* node) :
            NodeGraphOp(node, "Difference Normalized")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalDiffNorm() = default;
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
    class MathNumericalDiff : public NodeGraphOp {
    public:
        MathNumericalDiff(NodeGraphBase* node) :
            NodeGraphOp(node, "Difference")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalDiff() = default;
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