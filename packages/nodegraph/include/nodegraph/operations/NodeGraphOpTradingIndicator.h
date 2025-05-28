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
}

namespace l::nodegraph {


    /*********************************************************************/
    // https://www.quantifiedstrategies.com/volume-indicators/

    class TradingIndicatorOBV : public NodeGraphOp {
    public:
        TradingIndicatorOBV(NodeGraphBase* node) :
            NodeGraphOp(node, "OBV (on-balance volume)")
        {
            AddInput("Close", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Volume", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Friction", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("OBV", 0.0f);
        }

        virtual ~TradingIndicatorOBV() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto closeInput = inputs.at(0).GetIterator(numSamples);
            auto volInput = inputs.at(1).GetIterator(numSamples);
            auto friction = l::math::sqrt(inputs.at(2).Get());
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float close = *closeInput++;
                float vol = *volInput++;

                if (close > mClosePrev) {
                    mOBV += vol;
                }
                else if (close < mClosePrev) {
                    mOBV -= vol;
                }

                mOBV *= friction;

                mClosePrev = close;

                *output++ = mOBV;
            }

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mClosePrev = 0.0f;
                mOBV = 0.0f;
            }
        }

    protected:
        int32_t mReadSamples = 0;

        float mClosePrev = 0.0f;
        float mOBV = 0.0f;
    };

    /*********************************************************************/
    class TradingIndicatorVRSI : public NodeGraphOp {
    public:
        TradingIndicatorVRSI(NodeGraphBase* node) :
            NodeGraphOp(node, "VRSI (volume relative strength index)")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddOutput("RSI", 0.0f);
        }

        virtual ~TradingIndicatorVRSI() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = inputs.at(0).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = *input++;
                *output++ = in;
            }
        }

    protected:
    };

    /*********************************************************************/
    class TradingIndicatorVPT : public NodeGraphOp {
    public:
        TradingIndicatorVPT(NodeGraphBase* node) :
            NodeGraphOp(node, "VPT (volume-price trend)")
        {
            AddInput("Close", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Volume", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Friction", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("VPT", 0.0f);
        }

        virtual ~TradingIndicatorVPT() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto closeInput = inputs.at(0).GetIterator(numSamples);
            auto volInput = inputs.at(1).GetIterator(numSamples);
            auto friction = l::math::sqrt(inputs.at(2).Get());
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float close = *closeInput++;
                float vol = *volInput++;

                if (l::math::abs(mClosePrev) > 0.000000001f) {
                    mVPT += vol * (close - mClosePrev) / mClosePrev;
                }
                mVPT *= friction;

                mClosePrev = close;

                *output++ = mVPT;
            }

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mClosePrev = 0.0f;
                mVPT = 0.0f;
            }
        }

    protected:
        int32_t mReadSamples = 0;

        float mClosePrev = 0.0f;
        float mVPT = 0.0f;
    };

}

