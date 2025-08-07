#include "nodegraph/operations/NodeGraphOpUI.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* UI elements */

        /*********************************************************************/
    void GraphUICheckbox::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = inputs.at(0).GetIterator(1);
        auto output = outputs.at(0).GetIterator(1);

        if (mInputHasChanged) {
            *input = mInputState ? 1.0f : 0.0f;
            mInputHasChanged = false;
        }
        else {
            mInputState = *input;
        }
       
        *output = *input != 0.0f ? 1.0f : 0.0f;
    }

    bool& GraphUICheckbox::GetInputState() {
        return mInputState;
    }

    void GraphUICheckbox::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    /*********************************************************************/
    void GraphUISlider::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = inputs.at(0).GetIterator(1);
        auto min = inputs.at(1).GetIterator(1);
        auto max = inputs.at(2).GetIterator(1);
        auto power = inputs.at(3).GetIterator(1);
        auto output = outputs.at(0).GetIterator(1);

        mMin = *min;
        mMax = *max;

        if (mInputHasChanged) {
            *input = mInputValue;
            mInputHasChanged = false;
        }
        else {
            mInputValue = *input;
        }

        float length = mMax - mMin;
        if (length <= 0.0f) {
            *output = mMin;
        }
        float value = (mInputValue - mMin) / length;
        value = mMin + length * l::math::pow(value, *power);
        *output = l::math::clamp(value, mMin, mMax);
    }

    void GraphUISlider::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    float& GraphUISlider::GetMin() {
        return mMin;
    }

    float& GraphUISlider::GetMax() {
        return mMax;
    }

    float& GraphUISlider::GetInputValue() {
        return mInputValue;
    }

    /*********************************************************************/
    void GraphUIText::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        auto input = inputs.at(0).GetText(64);

        if (mInputHasChanged) {
            mOutputText.clear();
            mOutputText.append(input);
            mInputHasChanged = false;
        }
    }

    void GraphUIText::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    std::string_view GraphUIText::GetOutputText() {
        return mOutputText.str();
    }
    /*********************************************************************/
    void GraphUIChartLine::ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        int32_t mChannels = 2;

        outputs.at(0).MinimizeBuffer(numCacheSamples * mChannels);
        float* out = &outputs.at(0).Get(numCacheSamples * mChannels);

        float* input[2];
        for (int32_t j = 0; j < mChannels; j++) {
            input[j] = &inputs.at(j).Get(numSamples);
        }
        auto buf = out + writtenSamples * mChannels;
        int32_t j = 0;
        for (j = 0; j < numSamples; j++) {
            auto unixtimef = *input[0];
            auto unixtime = l::math::algorithm::convert<int32_t>(unixtimef);
            if (unixtimef == 0.0f || mLatestUnixtime >= unixtime) {
                //mLatestUnixtime = unixtime;
                //break;
            }
            else {
                mLatestUnixtime = unixtime;
            }
            for (int32_t i = 0; i < mChannels; i++) {
                *buf++ = *input[i]++;
            }
        }
        for (; j < numSamples; j++) {
            for (int32_t i = 0; i < mChannels; i++) {
                *buf++ = 0.0f;
            }
        }
    }

    /*********************************************************************/
    void GraphUICandleSticks::ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        const int32_t stride = 6;

        if (writtenSamples == 0) {
            outputs.at(0).MinimizeBuffer(numCacheSamples * stride);
            mLatestUnixtime = 0;
        }

        float* out = &outputs.at(0).Get(numCacheSamples * stride);

        float* input[stride];
        for (int32_t j = 0; j < stride; j++) {
            input[j] = &inputs.at(j).Get(numSamples);
        }
        auto buf = out + mWrittenSamples * stride;
        int32_t j = 0;
        for (j = 0; j < numSamples; j++) {
            auto unixtimef = *input[0];
            auto unixtime = l::math::algorithm::convert<int32_t>(unixtimef);
            if (unixtimef == 0.0f || mLatestUnixtime >= unixtime) {
                mLatestUnixtime = unixtime;
                break;
            }
            mLatestUnixtime = unixtime;
            for (int32_t i = 0; i < stride; i++) {
                *buf++ = *input[i]++;
            }
        }
        for (; j < numSamples; j++) {
            for (int32_t i = 0; i < stride; i++) {
                *buf++ = 0.0f;
            }
        }
    }

    /*********************************************************************/
    void GraphUIChartMarkers::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        auto timeInput = &inputs.at(0).Get(numSamples);
        auto openInput = &inputs.at(1).Get(numSamples);
        auto closeInput = &inputs.at(2).Get(numSamples);
        auto eventInput = &inputs.at(3).Get(numSamples);

        if (mReadSamples == 0) {
            mMarkers.clear();
        }

        auto beep = [&](int32_t unixtime, int32_t freq, int32_t duration) {
            if (mLastBeep + 30 < unixtime) {
                l::audio::PCBeep(freq, duration);
                mLastBeep = unixtime;
            }
            };

        for (int32_t i = 0; i < numSamples; i++) {
            auto time = *timeInput++;
            auto unixtime = l::math::algorithm::convert<int32_t>(time);
            auto open = *openInput++;
            auto close = *closeInput++;
            auto marker = *eventInput++;
            if (unixtime == 0) {
                mPrevValue = marker;
                continue;
            }
            auto price = (open + close) * 0.5f;
            auto currentMarkerChange = 1.0f;
            if (mPrevY > 0.0f) {
                currentMarkerChange = price / mPrevY;
            }
            bool hitStopLoss = false;
            if (mOrderPlaced) {
                auto meanStop = 0.0f;
                if (mProfitMean > 1.0f) {
                    auto meanStopFactor = 1.0f;
                    meanStop = 1.0f - meanStopFactor * (mProfitMean - 1.0f);
                }
                if (currentMarkerChange < meanStop) {
                    //hitStopLoss = true;
                }
                auto emaStop = 0.0f;
                if (mProfitEma > 1.0f) {
                    auto emaStopFactor = 1.0f;
                    emaStop = 1.0f - emaStopFactor * (mProfitEma - 1.0f);
                }
                if (currentMarkerChange < emaStop) {
                    //hitStopLoss = true;
                }
            }

            auto buyCond = marker > 0.0f && mPrevValue <= 0.0f;
            auto sellCond = marker < 0.0f && mPrevValue >= 0.0f;
            auto buy = !mOrderPlaced && !mStopLossActive && buyCond;
            auto sell = mOrderPlaced && !mStopLossActive && (hitStopLoss || sellCond);

            if (buy) {
                buySellCounter++;
                mMarkers.push_back({ unixtime, price, marker, mYTotalChange });
                mOrderPlaced = true;
                mPrevY = price;
                beep(unixtime, 4000, 50);
            }
            else if (sell) {
                buySellCounter--;
                if (mPrevY > 0.0f && price > 0.0f) {
                    mYTotalChange *= currentMarkerChange;
                    mProfitEma = mProfitEma + 0.5f * (currentMarkerChange - mProfitEma);
                    if (mMarkers.size() > 1) {
                        mProfitMean = 1.0f + (mYTotalChange - 1.0f) / static_cast<float>(mMarkers.size());
                    }
                }
                mMarkers.push_back({ unixtime, price, marker, mYTotalChange });
                mOrderPlaced = false;
                if (!mStopLossActive && hitStopLoss) {
                    // stop loss must not be active until this round and sell must not be 
                    mStopLossActive = true;
                }
                beep(unixtime, 1000, 50);
            }
            else if (mStopLossActive && sellCond) {
                // wait for actual sell signal so we can begin anew
                mStopLossActive = false;
            }
            mPrevValue = marker;
        }

        mReadSamples += numSamples;
        if (mReadSamples >= numCacheSamples) {
            mReadSamples = 0;
            mPrevValue = 0.0f;
            mYTotalChange = 1.0f;
            mOrderPlaced = false;
        }
    }
}
