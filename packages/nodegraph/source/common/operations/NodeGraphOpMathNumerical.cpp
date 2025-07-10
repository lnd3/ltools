#include "nodegraph/operations/NodeGraphOpMathNumerical.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    void MathNumericalIntegral::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void MathNumericalDerivate::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void MathNumericalDiffNorm::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void MathNumericalDiff::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void MathNumericalLevelTrigger::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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
                float levelMinMax = (inRangeFactorClamped - min) / minmaxRange;
                level = numLevels * levelMinMax;
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

}
