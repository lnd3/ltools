#include "nodegraph/operations/NodeGraphOpTradingFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {
    void TradingFilterFlipGate::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = &inputs.at(0).Get(numSamples);
        auto posTrigDelay = static_cast<int32_t>(inputs.at(1).Get());
        auto negTrigDelay = static_cast<int32_t>(inputs.at(2).Get());
        auto gateTrig = &outputs.at(0).Get(numSamples);
        auto gate = &outputs.at(1).Get(numSamples);

        for (int32_t i = 0; i < numSamples; i++) {
            auto in = (*input++);
            bool pos = in > 0.01f;
            bool neg = in < -0.01f;

            if (mGate && neg) {
                // going negative
                mGate = false;
                mNegHoldCount = 0;
            }
            if (!mGate && pos) {
                // going positive
                mGate = true;
                mPosHoldCount = 0;
            }

            if (mGate) {
                if (mPosHoldCount == posTrigDelay) {
                    *gateTrig++ = mGate ? 1.0f : -1.0f;
                }
                else {
                    *gateTrig++ = !mGate ? 1.0f : -1.0f;
                }
                mPosHoldCount++;
            }
            else {
                if (mNegHoldCount == negTrigDelay) {
                    *gateTrig++ = !mGate ? 1.0f : -1.0f;
                }
                else {
                    *gateTrig++ = mGate ? 1.0f : -1.0f;
                }
                mNegHoldCount++;
            }

            *gate++ = mGate ? 1.0f : -1.0f;
        }
    }

    /*********************************************************************/


    void TradingFilterPulseInfo::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = &inputs.at(0).Get(numSamples);
        auto maxFlips = l::math::max2(static_cast<int32_t>(inputs.at(1).Get() + 0.5f), 1);

        auto meanPosOut = &outputs.at(0).Get(numSamples);
        auto meanNegOut = &outputs.at(1).Get(numSamples);
        auto maxPosOut = &outputs.at(2).Get(numSamples);
        auto maxNegOut = &outputs.at(3).Get(numSamples);

        if (mPosPulseIntervalCount.empty()) {
            mPosPulseIntervalCount.push_back(0);
        }
        if (mNegPulseIntervalCount.empty()) {
            mNegPulseIntervalCount.push_back(0);
        }

        for (int32_t i = 0; i < numSamples; i++) {
            auto in = (*input++);

            bool reversal = (in > 0.0f) != (mValuePrev1 > 0.0f);
            if (reversal && in > 0.0f) {
                // positive pulse
                mPosPulseIntervalCount.push_back(0.0f);
                while (mPosPulseIntervalCount.size() > maxFlips) {
                    mPosPulseIntervalCount.erase(mPosPulseIntervalCount.begin());
                }
            }
            else if (reversal && in < 0.0f) {
                // negative pulse
                mNegPulseIntervalCount.push_back(0.0f);
                while (mNegPulseIntervalCount.size() > maxFlips) {
                    mNegPulseIntervalCount.erase(mNegPulseIntervalCount.begin());
                }
            }

            if (in > 0.0f) {
                mPosPulseIntervalCount.back() += 1.0f;
            }
            else {
                mNegPulseIntervalCount.back() += 1.0f;
            }

            mValuePrev1 = in;

            float meanPos = 0.0f;
            float meanNeg = 0.0f;
            float maxPos = 0.0f;
            float maxNeg = 0.0f;

            for (auto count : mPosPulseIntervalCount) {
                if (count > maxPos) {
                    maxPos = count;
                }
                meanPos += count;
            }
            for (auto count : mNegPulseIntervalCount) {
                if (count > maxNeg) {
                    maxNeg = count;
                }
                meanNeg += count;
            }

            *meanPosOut++ = meanPos / static_cast<float>(maxFlips);
            *meanNegOut++ = meanNeg / static_cast<float>(maxFlips);
            *maxPosOut++ = maxPos;
            *maxNegOut++ = maxNeg;
        }
    }

    /*********************************************************************/

    void TradingFilterVWMA::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        auto inputValue = &inputs.at(0).Get(numSamples);
        auto inputWeight = &inputs.at(1).Get(numSamples);
        float width = l::math::max2(inputs.at(2).Get(), 1.0f);
        float balance = inputs.at(3).Get();

        auto output = outputs.at(0).GetIterator(numSamples);

        int32_t widthInt = 1 + static_cast<int32_t>(width);
        int32_t bufferSize = widthInt;
        float widthFrac = width - l::math::floor(width);

        for (int32_t i = 0; i < numSamples; i++) {

            if (mFilterInit || mWidth != widthInt) {
                mWidth = widthInt;
                mFilterInit = false;
                mFilterStateIndex = 0;

                if (mFilterState.size() < bufferSize) {
                    mFilterState.resize(bufferSize);
                    mFilterWeight.resize(bufferSize);
                }
                for (int32_t j = 0; j < widthInt; j++) {
                    mFilterState[j] = *inputValue;
                    mFilterWeight[j] = *inputWeight;
                }
            }

            mFilterState[mFilterStateIndex] = *inputValue++;
            mFilterWeight[mFilterStateIndex] = *inputWeight++;
            mFilterStateIndex = (mFilterStateIndex + 1) % bufferSize; // buffer is 1 larger than the truncated filter size so we can smooth on the last one

            float outVal = 0.0;
            float balanceFactor = 1.0f - balance;
            float balanceDelta = balance / width;
            float balanceDivisorSum = (balanceDelta - balanceFactor) * (widthFrac);
            { // remove a part of the first sample of the sum as it is not part of the moving average
                outVal = mFilterWeight[mFilterStateIndex] * mFilterState[mFilterStateIndex] * ((balanceDelta - balanceFactor) * (widthFrac));
            }
            for (int32_t j = mFilterStateIndex; j < bufferSize; j++) {
                outVal += mFilterWeight[j] * mFilterState[j] * balanceFactor;
                balanceDivisorSum += mFilterWeight[j] * balanceFactor;
                balanceFactor += balanceDelta;
            }
            for (int32_t j = 0; j < mFilterStateIndex; j++) {
                outVal += mFilterWeight[j] * mFilterState[j] * balanceFactor;
                balanceDivisorSum += mFilterWeight[j] * balanceFactor;
                balanceFactor += balanceDelta;
            }

            auto signal = outVal / balanceDivisorSum;

            *output++ = signal;
        }
    }




}
