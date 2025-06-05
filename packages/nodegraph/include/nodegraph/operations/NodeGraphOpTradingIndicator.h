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
                auto volume = *volInput++;

                mOBV += close > mClosePrev ? volume : -volume;
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
    class TradingIndicatorVPT : public NodeGraphOp {
    public:
        TradingIndicatorVPT(NodeGraphBase* node) :
            NodeGraphOp(node, "VPT (volume-price trend)")
        {
            AddInput("Close", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Buy vol", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Sell vol", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Friction", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("VPT", 0.0f);
        }

        virtual ~TradingIndicatorVPT() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto closeInput = inputs.at(0).GetIterator(numSamples);
            auto volBuyInput = inputs.at(1).GetIterator(numSamples);
            auto volSellInput = inputs.at(2).GetIterator(numSamples);
            auto friction = l::math::sqrt(inputs.at(3).Get());
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float close = *closeInput++;
                float volBuy = *volBuyInput++;
                float volSell = *volSellInput++;

                float vol = l::math::abs(volBuy - volSell);
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

    /*********************************************************************/
    class TradingIndicatorOBV2 : public NodeGraphOp {
    public:
        TradingIndicatorOBV2(NodeGraphBase* node) :
            NodeGraphOp(node, "OBV2 (on-balance volume)")
        {
            AddInput("Buy vol", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Sell vol", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Friction", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("OBV", 0.0f);
        }

        virtual ~TradingIndicatorOBV2() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto volBuyInput = inputs.at(0).GetIterator(numSamples);
            auto volSellInput = inputs.at(1).GetIterator(numSamples);
            auto friction = l::math::sqrt(inputs.at(2).Get());
            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float volBuy = *volBuyInput++;
                float volSell = *volSellInput++;

                mOBV += volBuy - volSell;
                mOBV *= friction;

                *output++ = mOBV;
            }

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mOBV = 0.0f;
            }
        }

    protected:
        int32_t mReadSamples = 0;
        float mOBV = 0.0f;
    };


    /*********************************************************************/
    class TradingIndicatorGA: public NodeGraphOp {
    public:
        TradingIndicatorGA(NodeGraphBase* node) :
            NodeGraphOp(node, "GA (Gated Accumulation)")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Input Range", 1.0f, 1, 0.00001f, l::math::constants::FLTMAX);
            AddInput("+Gate", 0.0f, 1, 0.0f, 1.0f);
            AddInput("-Gate", 0.0f, 1, -1.0f, 0.0f);
            AddInput("Friction", 0.0f, 1, 0.0f, 1.0f);

            AddOutput("GA", 0.0f);
        }

        virtual ~TradingIndicatorGA() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto inInput = inputs.at(0).GetIterator(numSamples);
            auto inputRange = inputs.at(1).Get();
            auto gatePlus = inputs.at(2).Get();
            auto gateMinus = inputs.at(3).Get();
            auto friction = l::math::sqrt(inputs.at(4).Get());
            auto oneOverInputRange = 1.0f / inputRange;

            auto output = outputs.at(0).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = l::math::clamp(*inInput++, -inputRange, inputRange) * oneOverInputRange;

                if (!mAccMode && in > gatePlus) {
                    mAccMode = true;
                    mAcc = 0.0f;
                }else if (mAccMode && in < gateMinus) {
                    mAccMode = false;
                    mAcc = 0.0f;
                }

                if (mAccMode) {
                    mAcc += in - gatePlus;
                }
                else {
                    mAcc += in - gateMinus;
                }

                mAcc *= friction;

                *output++ = mAcc * oneOverInputRange;
            }

            mReadSamples += numSamples;

            if (mReadSamples >= numCacheSamples) {
                mReadSamples = 0;
                mInPrev = 0.0f;
                mAcc = 0.0f;
            }
        }

    protected:
        int32_t mReadSamples = 0;
        bool mAccMode = false;
        float mInPrev = 0.0f;
        float mAcc = 0.0f;
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
    class TradingIndicatorATR : public NodeGraphOp {
    public:
        TradingIndicatorATR(NodeGraphBase* node) :
            NodeGraphOp(node, "ATR (average true range)")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddOutput("ATR", 0.0f);
        }

        virtual ~TradingIndicatorATR() = default;
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
    class TradingIndicatorSD : public NodeGraphOp {
    public:
        TradingIndicatorSD(NodeGraphBase* node) :
            NodeGraphOp(node, "SD (standard deviation)")
        {
            AddInput("In", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddOutput("SD", 0.0f);
        }

        virtual ~TradingIndicatorSD() = default;
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
}