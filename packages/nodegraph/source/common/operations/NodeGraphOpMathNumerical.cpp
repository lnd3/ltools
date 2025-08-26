#include "nodegraph/operations/NodeGraphOpMathNumerical.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void MathNumericalIntegral::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input0 = &inputs.at(0).Get(numSamples);
        auto friction = inputs.at(1).Get();
        auto frictionFactor = l::math::clamp(l::math::pow(friction, 0.25f), 0.0f, 1.0f);
        auto output = &outputs.at(0).Get(numSamples);

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

    /*********************************************************************/
    void MathNumericalTemporalChange::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input0 = &inputs.at(0).Get(numSamples);
        auto output = &outputs.at(0).Get(numSamples);

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

    /*********************************************************************/
    void MathNumericalDiffNorm::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input0 = &inputs.at(0).Get(numSamples);
        auto output = &outputs.at(0).Get(numSamples);

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

    /*********************************************************************/
    void MathNumericalDiff::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input0 = &inputs.at(0).Get(numSamples);
        auto output = &outputs.at(0).Get(numSamples);

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

    /*********************************************************************/
    void MathNumericalLevelTrigger::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto inInput = &inputs.at(0).Get(numSamples);
        auto maxInput = inputs.at(1).GetIterator(numSamples);
        auto minInput = inputs.at(2).GetIterator(numSamples);
        auto numLevels = l::math::clamp(inputs.at(3).Get(), 1.0f, 10.0f);
        auto max = l::math::clamp(inputs.at(4).Get(), 0.0f, 3.0f);
        auto min = l::math::clamp(inputs.at(5).Get(), -3.0f, max);
        auto levelOutput = &outputs.at(0).Get(numSamples);
        auto pulseOutput = &outputs.at(1).Get(numSamples);

        for (int32_t i = 0; i < numSamples; i++) {
            float in = *inInput++;
            float inMax = *maxInput++;
            float inMin = *minInput++;
            float inRange = inMax - inMin;
            float inRangeFactor = 0.0f;
            if (inRange > 0.0f) {
                inRangeFactor = (in - inMin) / inRange;
            }
            float minmaxRange = max - min;
            float level = 0.0f;
            if (minmaxRange > 0.0f) {
                float levelMinMax = (inRangeFactor - min) / minmaxRange;
                level = levelMinMax;
            }
            auto levelExpanded = numLevels * level;

            float pulse = 0.0f;
            if (level >= 0.0f && level <= 1.0f && static_cast<int32_t>(levelExpanded) > static_cast<int32_t>(mLevelPrev)) {
                pulse = 1.0f;
                mLevelPrev = levelExpanded;
            }
            else if (level >= 0.0f && level <= 1.0f && static_cast<int32_t>(levelExpanded) < static_cast<int32_t>(mLevelPrev)) {
                pulse = -1.0f;
                mLevelPrev = levelExpanded;
            }

            *levelOutput++ = level;
            *pulseOutput++ = pulse;
        }
    }

    /*********************************************************************/
    void MathNumericalMinMaxChannel::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto inInput = &inputs.at(0).Get(numSamples);
        auto upperInput = inputs.at(1).GetIterator(numSamples);
        auto lowerInput = inputs.at(2).GetIterator(numSamples);
        auto friction = inputs.at(3).Get();

        auto rangeOutput = &outputs.at(0).Get(numSamples);
        auto rangeMaxOutput = &outputs.at(1).Get(numSamples);
        auto rangeMinOutput = &outputs.at(2).Get(numSamples);
        auto rangeNormOutput = &outputs.at(3).Get(numSamples);

        if (mReadSamples == 0) {
            mCurRangeMax = -100000000000000.0f;
            mCurRangeMin = 100000000000000.0f;
        }

        for (int32_t i = 0; i < numSamples; i++) {
            float in = *inInput++;
            float upper = *upperInput++;
            float lower = *lowerInput++;

            lower = lower > in ? in - 0.0000001f : lower;
            upper = upper < in ? in + 0.0000001f : upper;

            auto range = upper - lower;
            if (mCurRangeMax < range) {
                mCurRangeMax = range;
            }
            if (mCurRangeMin > range) {
                mCurRangeMin = range;
            }

            mCurRangeMax += friction * (range - mCurRangeMax);
            mCurRangeMin += friction * (range - mCurRangeMin);


            *rangeOutput++ = range;
            *rangeMaxOutput++ = mCurRangeMax;
            *rangeMinOutput++ = mCurRangeMin;

            auto rangeDiff = mCurRangeMax - mCurRangeMin;
            auto rangeNorm = 0.5f;
            if (rangeDiff > 0.0f) {
                rangeNorm = (range - mCurRangeMin) / rangeDiff;
            }

            *rangeNormOutput++ = rangeNorm;
        }

        mReadSamples += numSamples;

        if (mReadSamples >= numCacheSamples) {
            mReadSamples = 0;
        }
    }

    /*********************************************************************/
    void MathNumericalReconstructor::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        auto inInput = &inputs.at(0).Get(numSamples);
        auto baseInput = inputs.at(1).GetIterator();

        auto friction1 = inputs.at(2).Get();
        auto friction2 = inputs.at(3).Get();
        auto scale1 = inputs.at(4).Get();
        auto scale2 = inputs.at(5).Get();

        auto frictionFactor1 = l::math::clamp(l::math::pow(friction1, 0.25f), 0.0f, 1.0f);
        auto frictionFactor2 = l::math::clamp(l::math::pow(friction2, 0.25f), 0.0f, 1.0f);

        auto outputDiff = &outputs.at(0).Get(numSamples);
        auto outputDiffBase = &outputs.at(1).Get(numSamples);
        auto outputIntegral1 = &outputs.at(2).Get(numSamples);
        auto outputBase1 = &outputs.at(3).Get(numSamples);
        auto outputIntegral2 = &outputs.at(4).Get(numSamples);
        auto outputBase2 = &outputs.at(5).Get(numSamples);

        for (int32_t i = 0; i < numSamples; i++) {
            float in = *inInput++;
            auto base = *baseInput++;

            // derivate
            float diff = in - mInputPrev;
            mInputPrev = in;

            // integral 1
            mOutput1 += diff;
            mOutput1 *= frictionFactor1;

            // integral 2
            mOutput2 += (diff + mDiffPrev) * 0.5f;
            mOutput2 *= frictionFactor2;

            mDiffPrev = diff;

            auto outputScaled1 = mOutput1 * scale1;
            auto outputScaled2 = mOutput2 * scale2;

            *outputDiff++ = diff;
            *outputDiffBase++ = diff + base;
            *outputIntegral1++ = mOutput1;
            *outputBase1++ = outputScaled1 + base;
            *outputIntegral2++ = mOutput2;
            *outputBase2++ = outputScaled2 + base;
        }

        mReadSamples += numSamples;

        if (mReadSamples >= numCacheSamples) {
            mReadSamples = 0;
            mOutput1 = 0.0f;
            mOutput2 = 0.0f;
            mInputPrev = 0.0f;
            mDiffPrev = 0.0f;
        }
    }

    /*********************************************************************/
    void MathNumericalTrends::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        auto in1Input = &inputs.at(0).Get(numSamples);
        auto in2Input = &inputs.at(1).Get(numSamples);
        auto friction1 = inputs.at(2).Get();
        auto friction2 = inputs.at(3).Get();
        auto frictionFactor1 = l::math::clamp(l::math::pow(friction1, 0.25f), 0.0f, 1.0f);
        auto frictionFactor2 = l::math::clamp(l::math::pow(friction2, 0.25f), 0.0f, 1.0f);

        auto trend1Output = &outputs.at(0).Get(numSamples);
        auto trend2Output = &outputs.at(1).Get(numSamples);
        auto trendOutput = &outputs.at(2).Get(numSamples);

        auto in1Enabled = inputs.at(0).HasInputNode();
        auto in2Enabled = inputs.at(1).HasInputNode();

        for (int32_t i = 0; i < numSamples; i++) {
            float in1 = 0.0f;
            float in2 = 0.0f;

            if (in1Enabled) {
                in1 = *in1Input++;
            }
            if (in2Enabled) {
                in2 = *in2Input++;
            }

            auto in = (in1 + in2) * 0.5f;

            auto in1Diff = in1 - mIn1Prev1;
            auto in2Diff = in2 - mIn2Prev1;
            auto inDiff = in - mInPrev1;

            mIn1Accum += in1Diff;
            mIn1Accum *= frictionFactor2;
            mIn2Accum += in2Diff;
            mIn2Accum *= frictionFactor2;
            mInAccum += inDiff;
            mInAccum *= frictionFactor1;

            auto in1Accum = mIn1Accum - mIn1AccumPrev1;
            auto in2Accum = mIn2Accum - mIn2AccumPrev1;
            auto inAccum = mInAccum - mInAccumPrev1;

            *trend1Output++ = in1Accum;
            *trend2Output++ = in2Accum;
            *trendOutput++ = inAccum;

            mIn1Prev1 = in1;
            mIn2Prev1 = in2;
            mInPrev1 = in;

            mIn1AccumPrev1 = mIn1Accum;
            mIn2AccumPrev1 = mIn2Accum;
            mInAccumPrev1 = mInAccum;
        }

        mReadSamples += numSamples;

        if (mReadSamples >= numCacheSamples) {
            mReadSamples = 0;
        }
    }

}
