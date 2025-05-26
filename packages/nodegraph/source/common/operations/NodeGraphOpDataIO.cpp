#include "nodegraph/operations/NodeGraphOpDataIO.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void DataIODataIn::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, mInputDataStride * numSamples);

        float* input = mInputManager.GetArray(0);
        for (int32_t i = 0; i < mInputDataStride; i++) {
            float* output = &outputs.at(i).Get(numSamples);
            for (int32_t j = 0; j < numSamples; j++) {
                output[j] = input[mInputDataStride * j + i];
            }
        }
    }

    /*********************************************************************/
    void DataIODataOut::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);

        float* output = &outputs.at(0).Get(mOutputDataStride * numSamples);
        for (int32_t i = 0; i < numSamples; i++) {
            for (int32_t j = 0; j < mOutputDataStride; j++) {
                output[mOutputDataStride * i + j] = mInputManager.GetValueNext(j);
            }
        }
    }

    /*********************************************************************/
    void DataIOOCHLVDataIn::Reset() {
        mReadSamples = 0;
    }

    void DataIOOCHLVDataIn::InputHasChanged(int32_t numSamplesWritten) {
        mInputHasChanged = true;
        mWrittenSamples = numSamplesWritten;
        mReadSamples = 0;
    }

    int32_t DataIOOCHLVDataIn::GetNumSamplesLeft() {
        return mWrittenSamples - mReadSamples;
    }

    void DataIOOCHLVDataIn::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        int32_t stride = 11;

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
            auto in = &inputs.at(0).Get((numSamples + mReadSamples) * stride, mReadSamples * stride);

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
                *out2++ = in[offset + 1]; // open
                *out3++ = in[offset + 2]; // close
                *out4++ = in[offset + 3]; // high
                *out5++ = in[offset + 4]; // low
                *out6++ = in[offset + 5]; // volume
                *out7++ = in[offset + 6]; // quantity
                *out8++ = in[offset + 7]; // buy volume
                *out9++ = in[offset + 5] - in[offset + 7]; // sell volume
                *out10++ = in[offset + 9]; // buy quantity
                *out11++ = in[offset + 6] - in[offset + 9]; // sell quantity
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
    void DataIOPlaceTrade::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        auto now = l::string::get_unix_epoch();

        auto symbol = inputs.at(0).GetText(16);
        auto base = inputs.at(1).GetText(16);
        int32_t intervalMin = l::math::max2(static_cast<int32_t>(inputs.at(2).Get() + 0.5f), 1);
        auto unixtimeInput = &inputs.at(3).Get(numSamples);
        auto decisionInput = &inputs.at(4).Get(numSamples);
        auto convictionInput = &inputs.at(5).Get(numSamples);
        float indecisionLevel = inputs.at(6).Get();

        int32_t candleStartedAt = (60 * intervalMin) * (now / (60 * intervalMin));
        float candleProgress = (now - candleStartedAt) / static_cast<float>(60 * intervalMin);

        outputs.at(0).SetText(symbol);
        outputs.at(1).SetText(base);
        outputs.at(2).Get() = static_cast<float>(intervalMin);
        auto unixtimeOut = &outputs.at(3).Get(numSamples);
        auto decisionOut = &outputs.at(4).Get(numSamples);
        auto convictionOut = &outputs.at(5).Get(numSamples);
        outputs.at(6).Get() = candleProgress;
        outputs.at(7).Get() = indecisionLevel;

        if (mWrittenSamples < numCacheSamples) {
            for (int32_t i = 0; i < numSamples; i++) {
                auto time = unixtimeInput[i];
                auto unixtime = l::math::algorithm::convert<int32_t>(time);

                if (time == 0.0f || (mUnixtimePrev > 0 && unixtime < mUnixtimePrev)) {
                    unixtimeOut[i] = 0.0f;
                    decisionOut[i] = 0.0f;
                    convictionOut[i] = 0.0f;
                }
                else {
                    auto decision = decisionInput[i];
                    auto conviction = convictionInput[i];

                    unixtimeOut[i] = time;
                    decisionOut[i] = decision;
                    convictionOut[i] = conviction;

                    mUnixtimePrev = unixtime;
                }
            }
            mWrittenSamples += numSamples;
        }

        if (mWrittenSamples >= numCacheSamples) {
            mWrittenSamples = 0;
            mUnixtimePrev = 0;
        }
    }

    /*********************************************************************/
    void GraphCache::Reset() {
        mReadSamples = 0;
        mNode->ForEachInput(
            [&](NodeGraphInput& input) {
                if (input.HasInputNode() && input.GetInputNode()->IsOutOfDate()) {
                    mInputHasChanged = true;
                    mWrittenSamples = 0;
                }
            });
    }

    void GraphCache::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        if (numSamples > numCacheSamples) {
            numCacheSamples = numSamples;
        }

        if (!mInputHasChanged) {
            for (auto& in : inputs) {
                if (in.HasInputNode() && in.GetInputNode()->IsOutOfDate()) {
                    mInputHasChanged = true;
                    mWrittenSamples = 0;
                }
            }
        }

        if (mWrittenSamples < numCacheSamples) {
            mInputHasChanged = true;
            if (mBuffer.size() < numCacheSamples * mChannels) {
                mBuffer.resize(numCacheSamples * mChannels);
            }

            float* input[4];
            for (int32_t j = 0; j < mChannels; j++) {
                input[j] = &inputs.at(j).Get(numSamples);
            }
            auto buf = mBuffer.data() + mWrittenSamples * mChannels;
            for (int32_t j = 0; j < numSamples; j++) {
                for (int32_t i = 0; i < mChannels; i++) {
                    *buf++ = *(input[i])++;
                }
            }

            mWrittenSamples += numSamples;
        }

        if (mReadSamples < numCacheSamples) {
            float* output[4];
            for (int32_t j = 0; j < mChannels; j++) {
                output[j] = &outputs.at(j).Get(numSamples);
            }

            auto buf = mBuffer.data() + mReadSamples * mChannels;

            for (int32_t j = 0; j < numSamples; j++) {
                for (int32_t i = 0; i < mChannels; i++) {
                    *(output[i])++ = *buf++;
                }
            }

            mReadSamples += numSamples;
        }

        if (mWrittenSamples >= numCacheSamples) {
            mInputHasChanged = false;
        }
        if (mReadSamples >= numCacheSamples) {
            mReadSamples = 0;
        }
    }

}
