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
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto friction = inputs.at(1).Get();
            auto frictionFactor = l::math::clamp(l::math::pow(friction, 0.25f), 0.0f, 1.0f);
            auto lodExp = inputs.at(2).Get();
            auto lodFactor = l::math::pow(2.0f, l::math::round(lodExp));
            auto output = outputs.at(0).GetIterator(numSamples, lodFactor);

            for (int32_t i = 0; i < numSamples; i++) {
                mOutput += *input0++;
                mOutput *= frictionFactor;
                *output++ = mOutput;
            }

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mOutput = 0.0f;
            }

            if (isnan(mOutput)) {
                mOutput = 0.0f;
            }


        }
        virtual void Reset() override {
            mOutput = 0.0f;
        }
    protected:
        int32_t mReadSamples = 0;

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
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
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

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mInputPrev = 0.0f;
            }

        }
    protected:
        int32_t mReadSamples = 0;

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
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
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

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mInputPrev = 0.0f;
            }

        }
    protected:
        int32_t mReadSamples = 0;

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
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
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

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mInputPrev = 0.0f;
            }

        }
    protected:
        int32_t mReadSamples = 0;

        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalLevelTrigger : public NodeGraphOp {
    public:
        MathNumericalLevelTrigger(NodeGraphBase* node) :
            NodeGraphOp(node, "Level Trigger")
        {
            AddInput2("In", 1, InputFlags(false, false, false, false));
            AddInput2("Max", 1, InputFlags(false, false, false, false));
            AddInput2("Min", 1, InputFlags(false, false, false, false));
            AddInput("Num levels", 1.0f, 1, 1.0f, 10.0f, true, true);
            AddInput("Max%", 1.0f, 1, 0.0f, 1.0f, true, true);
            AddInput("Min%", 1.0f, 1, 0.0f, 1.0f, true, true);
            AddOutput("Level", 0.0f, 1);
            AddOutput("Pulse", 0.0f, 1);
        }

        virtual ~MathNumericalLevelTrigger() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto inInput = &inputs.at(0).Get(numSamples);
            auto maxInput = &inputs.at(1).Get();
            auto minInput = &inputs.at(2).Get();
            auto numLevels = l::math::clamp(inputs.at(3).Get(), 1.0f, 10.0f);
            auto max = l::math::clamp(inputs.at(4).Get(), 0.0f, 1.0f);
            auto min = l::math::clamp(inputs.at(5).Get(), 0.0f, max);
            auto levelOutput = &outputs.at(0).Get(numSamples);
            auto pulseOutput = &outputs.at(1).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = *inInput++;
                float inMax = *maxInput++;
                float inMin = *minInput++;
                float inLimited = l::math::clamp(in, inMin, inMax);
                float inRange = inMax - inMin;
                float inRangeFactor = 0.0f;
                if (inRange > 0.0f) {
                    inRangeFactor = (inLimited - inMin) / inRange;
                }
                float inRangeFactorClamped = l::math::clamp(inRangeFactor, min, max);
                float minmaxRange = max - min;
                float level = 0.0f;
                if (minmaxRange > 0.0f) {
                    level = numLevels * inRangeFactorClamped / minmaxRange;
                }
                float pulse = 0.0f;
                if (static_cast<int32_t>(mLevelPrev) != static_cast<int32_t>(level)) {
                    pulse = level > mLevelPrev ? 1.0f : -1.0f;
                }

                mLevelPrev = level;
                *levelOutput++ = level;
                *pulseOutput++ = pulse;
            }
        }
    protected:
        float mLevelPrev = 0.0f;
    };
}