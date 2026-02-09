#include "nodegraph/operations/NodeGraphOpUI.h"

#include "logging/Log.h"

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

        // Read Chart ID from input 3
        mChartId = static_cast<int32_t>(inputs.at(3).Get());

        outputs.at(0).MinimizeBuffer(numCacheSamples * mChannels);
        float* out = &outputs.at(0).Get(numCacheSamples * mChannels);

        float* input[2];
        for (int32_t j = 0; j < mChannels; j++) {
            input[j] = &inputs.at(j).Get(numSamples);
        }
        auto buf = out + writtenSamples * mChannels;
        int32_t j = 0;
        for (j = 0; j < numSamples; j++) {
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
    void GraphUIChartLine2::ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        int32_t mChannels = 3;

        // Read Chart ID from input 4
        mChartId = static_cast<int32_t>(inputs.at(4).Get());

        outputs.at(0).MinimizeBuffer(numCacheSamples * mChannels);
        float* out = &outputs.at(0).Get(numCacheSamples * mChannels);

        float* input[3];
        for (int32_t j = 0; j < mChannels; j++) {
            input[j] = &inputs.at(j).Get(numSamples);
        }
        auto buf = out + writtenSamples * mChannels;
        int32_t j = 0;
        for (j = 0; j < numSamples; j++) {
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
    void GraphUIChartLine3::ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        int32_t mChannels = 4;

        // Read Chart ID from input 5
        mChartId = static_cast<int32_t>(inputs.at(5).Get());

        outputs.at(0).MinimizeBuffer(numCacheSamples * mChannels);
        float* out = &outputs.at(0).Get(numCacheSamples * mChannels);

        float* input[4];
        for (int32_t j = 0; j < mChannels; j++) {
            input[j] = &inputs.at(j).Get(numSamples);
        }
        auto buf = out + writtenSamples * mChannels;
        int32_t j = 0;
        for (j = 0; j < numSamples; j++) {
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

        // Read Chart ID from input 7
        mChartId = static_cast<int32_t>(inputs.at(7).Get());

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
        auto entry1Input = &inputs.at(3).Get(numSamples);
        auto entry2Input = &inputs.at(4).Get(numSamples);
        auto slip = inputs.at(5).Get();
        //auto mainSize = inputs.at(10).Get();
        auto entry3Input = &inputs.at(11).Get(numSamples);

        auto entry1Active = inputs.at(3).HasInputNode();
        auto entry2Active = inputs.at(4).HasInputNode();
        auto entry3Active = inputs.at(11).HasInputNode();
        auto entryShared3 = ((entry1Active && entry2Active) || (entry1Active && entry3Active) || (entry2Active && entry3Active)) ? 0.5f : 1.0f;
        auto entryShare = (entry1Active && entry2Active && entry3Active) ? 0.3333f : entryShared3;

        if (mReadSamples == 0) {
            mMarkers.clear();
            mEntry1.Reset(entry1Active ? entryShare : 0.0f);
            mEntry2.Reset(entry2Active ? entryShare : 0.0f);
            mEntry3.Reset(entry3Active ? entryShare : 0.0f);
        }

        for (int32_t i = 0; i < numSamples; i++) {
            auto time = *timeInput++;
            auto unixtime = l::math::algorithm::convert<int32_t>(time);
            auto open = *openInput++;
            auto close = *closeInput++;
            auto entry1 = *entry1Input++;
            auto entry2 = *entry2Input++;
            auto entry3 = *entry3Input++;

            if (unixtime == 0) {
                continue;
            }

            auto estimatedPrice = (open + close) * 0.5f;

            mEntry1.Update(entry1, estimatedPrice, unixtime);
            mEntry2.Update(entry2, estimatedPrice, unixtime);
            mEntry3.Update(entry3, estimatedPrice, unixtime);

            if (mEntry1.TradeEntered(unixtime)) {
                auto s = std::make_tuple(unixtime, estimatedPrice, 1.0f, mTotalProfit);
                mMarkers.push_back(std::move(s));
            }
            if (mEntry2.TradeEntered(unixtime)) {
                auto s = std::make_tuple(unixtime, estimatedPrice, 1.0f, mTotalProfit);
                mMarkers.push_back(std::move(s));
            }
            if (mEntry3.TradeEntered(unixtime)) {
                auto s = std::make_tuple(unixtime, estimatedPrice, 1.0f, mTotalProfit);
                mMarkers.push_back(std::move(s));
            }
            if (mEntry1.TradeExited(unixtime)) {
                mTotalProfit *= mEntry1.GetProfit(slip);
                auto s = std::make_tuple(unixtime, estimatedPrice, -1.0f, mTotalProfit);
                mMarkers.push_back(std::move(s));
                mEntry1.Reset(entry1Active ? entryShare : 0.0f);
            }
            if (mEntry2.TradeExited(unixtime)) {
                mTotalProfit *= mEntry2.GetProfit(slip);
                auto s = std::make_tuple(unixtime, estimatedPrice, -1.0f, mTotalProfit);
                mMarkers.push_back(std::move(s));
                mEntry2.Reset(entry2Active ? entryShare : 0.0f);
            }
            if (mEntry3.TradeExited(unixtime)) {
                mTotalProfit *= mEntry3.GetProfit(slip);
                auto s = std::make_tuple(unixtime, estimatedPrice, -1.0f, mTotalProfit);
                mMarkers.push_back(std::move(s));
                mEntry3.Reset(entry3Active ? entryShare : 0.0f);
            }
        }

        mReadSamples += numSamples;
        if (mReadSamples >= numCacheSamples) {
            mReadSamples = 0;
            mTotalProfit = 1.0f;
        }
    }
}
