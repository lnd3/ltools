#include "nodegraph/operations/NodeGraphOpTradingDataIO.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void TradingDataIOOCHLVDataIn::Reset() {
        mReadSamples = 0;
    }

    void TradingDataIOOCHLVDataIn::InputHasChanged(int32_t numSamplesWritten) {
        mInputHasChanged = true;
        mWrittenSamples = numSamplesWritten;
        mReadSamples = 0;
    }

    int32_t TradingDataIOOCHLVDataIn::GetNumSamplesLeft() {
        return mWrittenSamples - mReadSamples;
    }

    void TradingDataIOOCHLVDataIn::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        int32_t stride = 9;
        
        if (numSamples > numCacheSamples) {
            numCacheSamples = numSamples;
        }

        if (mInputHasChanged) {
            auto symbolInput = inputs.at(1).GetText(16);
            auto baseInput = inputs.at(2).GetText(16);
            auto intervalInput = static_cast<int32_t>(l::math::clamp(inputs.at(3).Get(1), 0.0f, 9.0f));

            outputs.at(0).SetText(symbolInput);
            outputs.at(1).SetText(baseInput);
            float* intervalOut = &outputs.at(2).Get(1);
            *intervalOut = math::max2(1.0f, static_cast<float>(kIntervals[intervalInput]));
        }

        if (mReadSamples < mWrittenSamples) {
            auto in = &inputs.at(0).Get(numCacheSamples * stride, mReadSamples * stride);

            float* out1 = &outputs.at(3).Get(numSamples); // unixtime
            float* out2 = &outputs.at(4).Get(numSamples); // open
            float* out3 = &outputs.at(5).Get(numSamples); // close
            float* out4 = &outputs.at(6).Get(numSamples); // high
            float* out5 = &outputs.at(7).Get(numSamples); // low
            float* out6 = &outputs.at(8).Get(numSamples); // symbol volume
            float* out7 = &outputs.at(9).Get(numSamples); // quantity volume (usually usd or btc)
            float* out8 = &outputs.at(10).Get(numSamples); // buy symbol volume
            float* out9 = &outputs.at(11).Get(numSamples); // sell symbol volume
            float* out10 = &outputs.at(12).Get(numSamples); // buy quantity volume (usually usd or btc)
            float* out11 = &outputs.at(13).Get(numSamples); // sell quantity volume (usually usd or btc)

            auto intervalMinutes = static_cast<int32_t>(outputs.at(2).Get(1) + 0.5f);

            if (mMode == 0) {
                for (int32_t j = 0; j < numSamples; j++) {
                    auto offset = j * stride;

                    auto unixtimef = in[offset + 0];
                    auto unixtime = l::math::algorithm::convert<int32_t>(unixtimef);
                    if (mUnixtimePrev == 0) {
                        mUnixtimePrev = unixtime;
                    }
                    else if (unixtime != mUnixtimePrev + intervalMinutes * 60) {
                        unixtime = 0;
                        unixtimef = l::math::algorithm::convert<float>(unixtime);
                    }
                    else {
                        mUnixtimePrev = unixtime;
                    }

                    *out1++ = unixtimef; // unixtime
                    auto o = in[offset + 1];
                    auto c = in[offset + 2];
                    auto h = in[offset + 3];
                    auto l = in[offset + 4];
                    auto v = in[offset + 5];

                    *out2++ = o;
                    *out3++ = c;
                    *out4++ = h;
                    *out5++ = l;
                    *out6++ = v;
                    *out7++ = in[offset + 6]; // quantity
                    *out8++ = in[offset + 7]; // buy volume
                    *out9++ = in[offset + 5] - in[offset + 7]; // sell volume
                    *out10++ = in[offset + 8]; // buy quantity
                    *out11++ = in[offset + 6] - in[offset + 8]; // sell quantity
                }
            }
            else if (mMode == 1) {
                for (int32_t j = 0; j < numSamples; j++) {
                    auto offset = j * stride;

                    auto unixtimef = in[offset + 0];
                    auto unixtime = l::math::algorithm::convert<int32_t>(unixtimef);
                    if (mUnixtimePrev == 0) {
                        mUnixtimePrev = unixtime;
                    }
                    else if (unixtime != mUnixtimePrev + intervalMinutes * 60) {
                        unixtime = 0;
                        unixtimef = l::math::algorithm::convert<float>(unixtime);
                    }
                    else {
                        mUnixtimePrev = unixtime;
                    }

                    *out1++ = unixtimef; // unixtime
                    auto o = in[offset + 1];
                    auto c = in[offset + 2];
                    auto h = in[offset + 3];
                    auto l = in[offset + 4];

                    auto close = 0.25f * (o + c + h + l);
                    auto open = 0.5f * (mOpenPrev + mClosePrev);
                    auto high = l::math::max3(h, close, open);
                    auto low = l::math::min3(l, close, open);

                    mOpenPrev = open;
                    mClosePrev = close;

                    *out2++ = open;
                    *out3++ = close;
                    *out4++ = high;
                    *out5++ = low;
                    *out6++ = in[offset + 5];
                    *out7++ = in[offset + 6]; // quantity
                    *out8++ = in[offset + 7]; // buy volume
                    *out9++ = in[offset + 5] - in[offset + 7]; // sell volume
                    *out10++ = in[offset + 8]; // buy quantity
                    *out11++ = in[offset + 6] - in[offset + 8]; // sell quantity
                }
            }

            mReadSamples += numSamples;
        }

        if (mReadSamples >= mWrittenSamples) {
            mInputHasChanged = false;
            mReadSamples = 0;
            mUnixtimePrev = 0;
        }
    }

    /*********************************************************************/
    void TradingDataIOPlaceTrade::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        auto symbol = inputs.at(0).GetText(16);
        auto base = inputs.at(1).GetText(16);
        int32_t intervalMin = l::math::max2(static_cast<int32_t>(inputs.at(2).Get() + 0.5f), 1);
        auto unixtimeInput = &inputs.at(3).Get(numSamples);
        auto longInput = &inputs.at(4).Get(numSamples);
        auto shortInput = &inputs.at(5).Get(numSamples);
        float indecisionLevel = inputs.at(6).Get();
        trade.setIndecisionLevel(indecisionLevel);

        outputs.at(0).SetText(symbol);
        outputs.at(1).SetText(base);
        outputs.at(2).Get() = static_cast<float>(intervalMin);
        auto unixtimeOut = &outputs.at(3).Get(numSamples);
        auto trendOut = &outputs.at(4).Get(numSamples);
        auto convictionOut = &outputs.at(5).Get(numSamples);
        auto reversalOut = &outputs.at(6).Get(numSamples);

        if (mWrittenSamples < numCacheSamples) {
            for (int32_t i = 0; i < numSamples; i++) {
                auto time = unixtimeInput[i];
                auto unixtime = l::math::algorithm::convert<int32_t>(time);

                if (time == 0.0f || (mUnixtimePrev > 0 && unixtime < mUnixtimePrev)) {
                    unixtimeOut[i] = 0.0f;
                    trendOut[i] = 0.0f;
                    convictionOut[i] = 0.0f;
                }
                else {
                    unixtimeOut[i] = time;
                    trade.setLongLevel(longInput[i]);
                    trade.setShortLevel(shortInput[i]);

                    trendOut[i] = trade.trend();
                    convictionOut[i] = trade.trendConviction();
                    reversalOut[i] = trade.isTrendReversal();

                    mUnixtimePrev = unixtime;

                    trade.step(i);
                }
            }
            mWrittenSamples += numSamples;
        }

        if (mWrittenSamples >= numCacheSamples) {
            mWrittenSamples = 0;
            mUnixtimePrev = 0;
            trade.clear();
        }
    }

}
