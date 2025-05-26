#include "nodegraph/operations/NodeGraphOpFuzzyFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {
    void FuzzyFilterFlipGate::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = &inputs.at(0).Get(numSamples);
        auto trigger = static_cast<int32_t>(inputs.at(1).Get());
        auto hold = static_cast<int32_t>(inputs.at(2).Get());
        auto gate = &outputs.at(0).Get(numSamples);
        auto gateHold = &outputs.at(1).Get(numSamples);

        for (int32_t i = 0; i < numSamples; i++) {
            auto in = (*input++);
            bool pos = in > 0.01f;
            bool neg = in < -0.01f;

            if (mGate && neg) {
                mGate = false;
                if (mTriggerCounter <= 0) {
                    mHoldCounter = hold;
                }
                mTriggerCounter = trigger;
            }
            if (!mGate && pos) {
                mGate = true;
                if (mTriggerCounter <= 0) {
                    mHoldCounter = hold;
                }
                mTriggerCounter = trigger;
            }

            if (mHoldCounter > 0) {
                *gate++ = !mGate ? 1.0f : -1.0f;
            }
            else {
                *gate++ = mGate ? 1.0f : -1.0f;
            }

            *gateHold++ = mGate ? 1.0f : -1.0f;

            mHoldCounter--;
            mTriggerCounter--;
        }
    }



    void FuzzyFilterPulseInfo::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

}
