#include "nodegraph/operations/NodeGraphOpTradingDataIO.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include <math/MathFunc.h>

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void TradingDataIOOCHLVDataIn::ProcessWriteCached(int32_t, int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        if (mInputHasChanged) {
            auto symbolInput = inputs.at(1).GetText(16);
            auto baseInput = inputs.at(2).GetText(16);
            auto intervalInput = static_cast<int32_t>(l::math::clamp(inputs.at(3).Get(1), 0.0f, 9.9999f));

            outputs.at(0).SetText(symbolInput);
            outputs.at(1).SetText(baseInput);
            float* intervalOut = &outputs.at(2).Get(1);
            *intervalOut = l::math::max2(1.0f, static_cast<float>(kIntervals[intervalInput]));
        }
    }

    void TradingDataIOOCHLVDataIn::ProcessReadCached(int32_t readSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        if (readSamples == 0) {
            mUnixtimePrev = 0;
        }

        int32_t stride = 9;

        inputs.at(0).MinimizeBuffer(numCacheSamples * stride);
        auto in = &inputs.at(0).Get(numCacheSamples * stride, readSamples * stride);
        auto timeframeMultiplier = static_cast<int32_t>(inputs.at(4).Get());

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
                else if (unixtime == mUnixtimePrev) {
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
                auto v = in[offset + 5]; // total volume
                auto q = in[offset + 6]; // total quantity
                auto bv = in[offset + 7]; // buy volume
                auto bq = in[offset + 8]; // buy quantity

                auto timemin = unixtime / 60;
                if (timemin % timeframeMultiplier == 0) {
                    // time interval looping so reset moving averages
                    mOpenMa = o; // simply first value
                    mCloseMa = c;
                    mHighMa = h;
                    mLowMa = l;
                    mVolMa = v;
                    mQuantMa = q;
                    mBuyVolMa = bv;
                    mBuyQuantMa = bq;
                }
                else {
                    // last value
                    mCloseMa = c;

                    // extremes
                    mHighMa = l::math::max2(mHighMa, h);
                    mLowMa = l::math::min2(mLowMa, l);

                    // just the sums
                    mVolMa += v;
                    mQuantMa += q;
                    mBuyVolMa += bv;
                    mBuyQuantMa += bq;
                }

                *out2++ = mOpenMa;
                *out3++ = mCloseMa;
                *out4++ = mHighMa;
                *out5++ = mLowMa;
                *out6++ = mVolMa;
                *out7++ = mQuantMa; // quantity

                *out8++ = mBuyVolMa; // buy volume
                *out9++ = mVolMa - mBuyVolMa; // sell volume
                *out10++ = mBuyQuantMa; // buy quantity
                *out11++ = mQuantMa - mBuyQuantMa; // sell quantity
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
    }

    void TradingDataIOChartInfo::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto symbolInput = inputs.at(0).GetText(16);
        auto baseInput = inputs.at(1).GetText(16);
        auto indexInput = l::math::clamp(inputs.at(2).Get(), 0.0f, 9.9999f);
        auto now = inputs.at(3).Get();

        outputs.at(0).SetText(symbolInput);
        outputs.at(1).SetText(baseInput);
        float* indexOut0 = &outputs.at(2).Get();
        float* indexOut1 = &outputs.at(3).Get();
        float* indexOut2 = &outputs.at(4).Get();
        float* indexOut3 = &outputs.at(5).Get();
        float* nowOutput = &outputs.at(6).Get();
        *indexOut0 = l::math::clamp(indexInput, 0.0f, 9.9999f);
        *indexOut1 = l::math::clamp(indexInput + 1.0f, 0.0f, 9.9999f);
        *indexOut2 = l::math::clamp(indexInput + 2.0f, 0.0f, 9.9999f);
        *indexOut3 = l::math::clamp(indexInput + 3.0f, 0.0f, 9.9999f);
        *nowOutput = now;
    }

}
