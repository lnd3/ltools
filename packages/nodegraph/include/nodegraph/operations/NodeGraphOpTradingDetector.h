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


namespace {
    class TrendDetectorBasic {
    public:
        TrendDetectorBasic() = default;
        ~TrendDetectorBasic() = default;

        float process(float in) {
            bool bullishTip = in > inPrev1;
            bool bullishTipTwice = in > inPrev2;
            bool bullishTipThrice = in > inPrev3;

            inPrev3 = inPrev2;
            inPrev2 = inPrev1;
            inPrev1 = in;

            float trend = (bullishTipThrice ? 0.3f : -0.3f) + (bullishTipTwice ? 0.35f : -0.35f) + (bullishTip ? 0.45f : -0.45f);
            return trend;
        }

    protected:
        float inPrev1 = 0.0f;
        float inPrev2 = 0.0f;
        float inPrev3 = 0.0f;
    };

    class TrendDetectorMean {
    public:
        TrendDetectorMean() = default;
        ~TrendDetectorMean() = default;

        float process(float in, int32_t numSamples = 6) {
            if (mHistory.size() != numSamples) {
                mHistory.resize(l::math::max2(numSamples, 1));
            }

            float mean = 0.0f;
            float factor = 1.0f / (mHistory.size() + 1);
            float acc = factor;
            for (auto value : mHistory) {
                mean += value * acc;
                acc += factor;
            }
            mean += in;
            mean = mean * factor;

            bool bullishLevel = in > mean;

            mHistory.erase(mHistory.begin());
            mHistory.push_back(in);

            float trend = bullishLevel ? 1.0f : -1.0f;
            return trend;
        }

    protected:
        std::vector<float> mHistory;
        float inPrev1 = 0.0f;
        float inPrev2 = 0.0f;
    };

    class ReversalDetector4x {
    public:
        ReversalDetector4x() = default;
        ~ReversalDetector4x() = default;

        float process(float in) {
            float diff01 = in - prevValue1;
            float diff12 = prevValue1 - prevValue2;
            float diff23 = prevValue2 - prevValue3;
            float diff34 = prevValue3 - prevValue4;

            bool bull1 = diff01 > 0.0f;
            bool bear1 = diff01 < 0.0f;

            bool troph1 = bull1 && diff12 < 0.0f;
            bool troph2 = troph1 && diff23 < 0.0f;
            bool troph3 = troph2 && diff34 < 0.0f;

            bool peak1 = bear1 && diff12 > 0.0f;
            bool peak2 = peak1 && diff23 > 0.0f;
            bool peak3 = peak2 && diff34 > 0.0f;

            float troph = (troph1 ? 1.0f : 0.0f) + (troph2 ? 1.0f : 0.0f) + (troph3 ? 1.0f : 0.0f);
            float peak = (peak1 ? 1.0f : 0.0f) + (peak2 ? 1.0f : 0.0f) + (peak3 ? 1.0f : 0.0f);

            float reversal = (troph - peak) * 0.33f;

            prevValue4 = prevValue3;
            prevValue3 = prevValue2;
            prevValue2 = prevValue1;
            prevValue1 = in;

            return reversal;
        }

    protected:
        float prevValue1 = 0.0f;
        float prevValue2 = 0.0f;
        float prevValue3 = 0.0f;
        float prevValue4 = 0.0f;
    };

    class ReversalDetector {
    public:
        ReversalDetector() = default;
        ~ReversalDetector() = default;

        float process(float in) {
            float diff01 = in - prevValue1;
            float diff12 = prevValue1 - prevValue2;

            bool bull1 = diff01 > 0.0f;
            bool bear1 = diff01 < 0.0f;

            bool troph1 = bull1 && diff12 < 0.0f;

            bool peak1 = bear1 && diff12 > 0.0f;

            float troph = troph1 ? 1.0f : 0.0f;
            float peak = peak1 ? 1.0f : 0.0f;

            float reversal = troph - peak;

            prevValue2 = prevValue1;
            prevValue1 = in;

            return reversal;
        }

    protected:
        float prevValue1 = 0.0f;
        float prevValue2 = 0.0f;
    };

    class AccelerationDetector {
    public:
        AccelerationDetector() = default;
        ~AccelerationDetector() = default;

        float process(float in) {
            float diff01 = in - prevValue1;
            float diff12 = prevValue1 - prevValue2;
            float diff23 = prevValue2 - prevValue3;
            float diff34 = prevValue3 - prevValue4;

            bool bull1 = diff01 > 0.0f;
            bool bear1 = diff01 < 0.0f;

            bool bullish1 = bull1 && diff01 > diff12;
            bool bullish2 = bullish1 && diff12 > diff23;
            bool bullish3 = bullish2 && diff23 > diff34;

            bool bearish1 = bear1 && diff01 < diff12;
            bool bearish2 = bearish1 && diff12 < diff23;
            bool bearish3 = bearish2 && diff23 < diff34;

            float bullish = (bullish1 ? 1.0f : 0.0f) + (bullish2 ? 1.0f : 0.0f) + (bullish3 ? 1.0f : 0.0f);
            float bearish = (bearish1 ? 1.0f : 0.0f) + (bearish2 ? 1.0f : 0.0f) + (bearish3 ? 1.0f : 0.0f);

            float acceleration = (bullish - bearish) * 0.33f;

            prevValue4 = prevValue3;
            prevValue3 = prevValue2;
            prevValue2 = prevValue1;
            prevValue1 = in;

            return acceleration;
        }

    protected:
        float prevValue1 = 0.0f;
        float prevValue2 = 0.0f;
        float prevValue3 = 0.0f;
        float prevValue4 = 0.0f;
    };
}

namespace l::nodegraph {

    /*********************************************************************/
    class TradingDetectorTrend : public NodeGraphOp {
    public:
        TradingDetectorTrend(NodeGraphBase* node) :
            NodeGraphOp(node, "Trend Detector")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Trend Samples", 6.0f, 1, 1.0f, 50.0f, false, false);

            AddOutput("Trend Basic", 0.0f);
            AddOutput("Trend Mean", 0.0f);
            AddOutput("Reversal", 0.0f);
            AddOutput("Accel", 0.0f);
            AddOutput("Sum", 0.0f);
        }

        virtual ~TradingDetectorTrend() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto numTrendSamples = static_cast<int32_t>(l::math::max2(inputs.at(1).Get(), 1.0f));

            auto outputTrendBasic = outputs.at(0).GetIterator(numSamples);
            auto outputTrendMean = outputs.at(1).GetIterator(numSamples);
            auto outputReversal = outputs.at(2).GetIterator(numSamples);
            auto outputAccel = outputs.at(3).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = (*input1++);

                auto trendBasic = mTrendBasic.process(in);
                auto trendMean = mTrendMean.process(in, numTrendSamples);
                auto reversal = mReversal.process(in);
                auto acceleration = mAcceleration.process(in);

                *outputTrendBasic++ = trendBasic;
                *outputTrendMean++ = trendMean;
                *outputReversal++ = reversal;
                *outputAccel++ = acceleration;
            }
        }

    protected:
        TrendDetectorBasic mTrendBasic;
        TrendDetectorMean mTrendMean;
        ReversalDetector4x mReversal;
        AccelerationDetector mAcceleration;
    };

    /*********************************************************************/
    class TradingDetectorTrendDiff : public NodeGraphOp {
    public:
        TradingDetectorTrendDiff(NodeGraphBase* node) :
            NodeGraphOp(node, "Trend Difference Detector")
        {
            AddInput("In 1", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("In 2", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Trend Samples", 6.0f, 1, 1.0f, 50.0f, false, false);

            AddOutput("Trend", 0.0f);
            AddOutput("Reversal", 0.0f);
            AddOutput("Accel", 0.0f);
            AddOutput("Sum", 0.0f);
        }

        virtual ~TradingDetectorTrendDiff() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto inputHifi = inputs.at(0).GetIterator(numSamples);
            auto inputLofi = inputs.at(1).GetIterator(numSamples);

            auto outputTrendBasic = outputs.at(0).GetIterator(numSamples);
            auto outputReversal = outputs.at(1).GetIterator(numSamples);
            auto outputAccel = outputs.at(2).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float inHifi = (*inputHifi++);
                float inLofi = (*inputLofi++);
                float inDiff = inHifi - inLofi;

                auto trendBasic = mTrend.process(inDiff);
                auto reversal = mReversal.process(inDiff);
                auto acceleration = mAcceleration.process(inDiff);

                *outputTrendBasic++ = trendBasic;
                *outputReversal++ = reversal;
                *outputAccel++ = acceleration;
            }
        }

    protected:
        TrendDetectorBasic mTrend;
        ReversalDetector4x mReversal;
        AccelerationDetector mAcceleration;
    };

}

