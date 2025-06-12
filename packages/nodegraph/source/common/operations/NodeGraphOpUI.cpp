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
            *input = mState ? 1.0f : 0.0f;
            mInputHasChanged = false;
        }
        else {
            mState = *input;
        }
       
        *output = *input != 0.0f ? 1.0f : 0.0f;
    }

    bool& GraphUICheckbox::GetStatePtr() {
        return mState;
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
            *input = mValue;
            mInputHasChanged = false;
        }
        else {
            mValue = *input;
        }

        float length = mMax - mMin;
        if (length <= 0.0f) {
            *output = mMin;
        }
        float value = (mValue - mMin) / length;
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

    float& GraphUISlider::GetValue() {
        return mValue;
    }

    /*********************************************************************/
    void GraphUIChartLine::Reset() {
        mNode->ForEachInput(
            [&](NodeGraphInput& input) {
                if (input.HasInputNode() && input.GetInputNode()->IsOutOfDate()) {
                    mInputHasChanged = true;
                    mWrittenSamples = 0;
                }
            });
    }

    int32_t GraphUIChartLine::GetNumSamplesLeft() {
        return mWrittenSamples;
    }

    void GraphUIChartLine::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        int32_t mChannels = 2;

        if (numSamples > numCacheSamples) {
            numCacheSamples = numSamples;
        }

        if (!mInputHasChanged) {
            for (auto& in : inputs) {
                if (in.HasInputNode() && in.GetInputNode()->IsOutOfDate()) {
                    mInputHasChanged = true;
                    mWrittenSamples = 0;
                    mLatestUnixtime = 0;
                }
            }
        }

        float* out = &outputs.at(0).Get(numCacheSamples * mChannels);

        if (mWrittenSamples < numCacheSamples) {
            float* input[2];
            for (int32_t j = 0; j < mChannels; j++) {
                input[j] = &inputs.at(j).Get(numSamples);
            }
            auto buf = out + mWrittenSamples * mChannels;
            int32_t j = 0;
            for (j = 0; j < numSamples; j++) {
                auto unixtimef = *input[0];
                auto unixtime = l::math::algorithm::convert<int32_t>(unixtimef);
                if (unixtimef == 0.0f || mLatestUnixtime >= unixtime) {
                    mLatestUnixtime = unixtime;
                    break;
                }
                mLatestUnixtime = unixtime;
                for (int32_t i = 0; i < mChannels; i++) {
                    *buf++ = *input[i]++;
                }
            }
            for (; j < numSamples; j++) {
                for (int32_t i = 0; i < mChannels; i++) {
                    *buf++ = 0.0f;
                }
            }

            mWrittenSamples += numSamples;
        }

        if (mWrittenSamples >= numCacheSamples) {
            mInputHasChanged = false;
        }
    }

    /*********************************************************************/
    void GraphUICandleSticks::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        if (!mInputHasChanged) {
            for (auto& in : inputs) {
                if (in.HasInputNode() && in.GetInputNode()->IsOutOfDate()) {
                    mInputHasChanged = true;
                    mWrittenSamples = 0;
                    mLatestUnixtime = 0;
                }
            }
        }

        const int32_t stride = 6;
        float* out = &outputs.at(0).Get(numCacheSamples * stride);

        if (mWrittenSamples < numCacheSamples) {
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

            mWrittenSamples += numSamples;
        }

        if (mWrittenSamples >= numCacheSamples) {
            mInputHasChanged = false;
        }
    }

}
