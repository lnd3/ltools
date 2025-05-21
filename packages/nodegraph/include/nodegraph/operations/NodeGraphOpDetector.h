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
    class TrendDetector {
    public:
        TrendDetector() = default;
        ~TrendDetector() = default;

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
            bool bullishTip = in > mHistory.front();
            bool bullishMean = mean > meanPrev;

            meanPrev = mean;
            mHistory.erase(mHistory.begin());
            mHistory.push_back(in);

            float trend = (bullishLevel ? 0.4f : -0.4f) + (bullishMean ? 0.35f : -0.35f) + (bullishTip ? 0.25f : -0.25f);
            return trend;
        }

    protected:
        std::vector<float> mHistory;
        float meanPrev;
    };

    class ReversalDetector {
    public:
        ReversalDetector() = default;
        ~ReversalDetector() = default;

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
    class GraphNumericDetector : public NodeGraphOp {
    public:
        GraphNumericDetector(NodeGraphBase* node) :
            NodeGraphOp(node, "Detector")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Trend Samples", 6.0f, 1, 1.0f, 50.0f, false, false);

            AddOutput("Trend", 0.0f);
            AddOutput("Reversal", 0.0f);
            AddOutput("Accel", 0.0f);
            AddOutput("Sum", 0.0f);
        }

        virtual ~GraphNumericDetector() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto numTrendSamples = static_cast<int32_t>(l::math::max2(inputs.at(1).Get(), 1.0f));

            auto outputTrend = outputs.at(0).GetIterator(numSamples);
            auto outputReversal = outputs.at(1).GetIterator(numSamples);
            auto outputAccel = outputs.at(2).GetIterator(numSamples);
            auto outputSum = outputs.at(3).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = (*input1++);

                auto trend = mTrend.process(in, numTrendSamples);
                auto reversal = mReversal.process(in);
                auto acceleration = mAcceleration.process(in);

                *outputTrend++ = trend;
                *outputReversal++ = reversal;
                *outputAccel++ = acceleration;

                *outputSum++ = l::math::min2((trend + reversal + acceleration) * 0.3333334f, 1.0f);
            }
        }

    protected:
        TrendDetector mTrend;
        ReversalDetector mReversal;
        AccelerationDetector mAcceleration;
    };

    /*********************************************************************/
    class GraphNumericDifferenceDetector : public NodeGraphOp {
    public:
        GraphNumericDifferenceDetector(NodeGraphBase* node) :
            NodeGraphOp(node, "Differential Detector")
        {
            AddInput("In 1", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("In 2", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Trend Samples", 6.0f, 1, 1.0f, 50.0f, false, false);

            AddOutput("Trend", 0.0f);
            AddOutput("Reversal", 0.0f);
            AddOutput("Accel", 0.0f);
            AddOutput("Sum", 0.0f);
        }

        virtual ~GraphNumericDifferenceDetector() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto inputHifi = inputs.at(0).GetIterator(numSamples);
            auto inputLofi = inputs.at(1).GetIterator(numSamples);
            auto numTrendSamples = static_cast<int32_t>(l::math::max2(inputs.at(2).Get(), 1.0f));

            auto outputTrend = outputs.at(0).GetIterator(numSamples);
            auto outputReversal = outputs.at(1).GetIterator(numSamples);
            auto outputAccel = outputs.at(2).GetIterator(numSamples);
            auto outputSum = outputs.at(3).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float inHifi = (*inputHifi++);
                float inLofi = (*inputLofi++);
                float inDiff = inHifi - inLofi;

                auto trend = mTrend.process(inDiff, numTrendSamples);
                auto reversal = mReversal.process(inDiff);
                auto acceleration = mAcceleration.process(inDiff);

                *outputTrend++ = trend;
                *outputReversal++ = reversal;
                *outputAccel++ = acceleration;
                *outputSum++ = (trend + reversal + acceleration) * 0.33334f;
            }
        }

    protected:
        TrendDetector mTrend;
        ReversalDetector mReversal;
        AccelerationDetector mAcceleration;
    };

}

