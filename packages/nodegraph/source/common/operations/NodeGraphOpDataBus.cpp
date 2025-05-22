#include "nodegraph/operations/NodeGraphOpDataBus.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphDataBusDataIn::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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
    void GraphDataBusDataOut::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);

        float* output = &outputs.at(0).Get(mOutputDataStride * numSamples);
        for (int32_t i = 0; i < numSamples; i++) {
            for (int32_t j = 0; j < mOutputDataStride; j++) {
                output[mOutputDataStride * i + j] = mInputManager.GetValueNext(j);
            }
        }
    }

    /*********************************************************************/
    void GraphDataOCHLVDataIn::Reset() {
        mReadSamples = 0;
    }

    void GraphDataOCHLVDataIn::InputHasChanged(int32_t numSamplesWritten) {
        mInputHasChanged = true;
        mWrittenSamples = numSamplesWritten;
    }

    int32_t GraphDataOCHLVDataIn::GetNumSamplesLeft() {
        return mWrittenSamples - mReadSamples;
    }

    void GraphDataOCHLVDataIn::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto in = &inputs.at(0).Get(numSamples * 6 + mReadSamples * 6, mReadSamples * 6);

        float* out1 = &outputs.at(3).Get(numSamples);
        float* out2 = &outputs.at(4).Get(numSamples);
        float* out3 = &outputs.at(5).Get(numSamples);
        float* out4 = &outputs.at(6).Get(numSamples);
        float* out5 = &outputs.at(7).Get(numSamples);
        float* out6 = &outputs.at(8).Get(numSamples);

        if (mInputHasChanged) {
            auto symbolInput = inputs.at(1).GetText(16);
            auto baseInput = inputs.at(2).GetText(16);
            auto intervalInput = static_cast<int32_t>(l::math::clamp(inputs.at(3).Get(1), 0.0f, 9.0f));

            outputs.at(0).SetText(symbolInput);
            outputs.at(1).SetText(baseInput);
            float* intervalOut = &outputs.at(2).Get(1);
            *intervalOut = math::max2(1.0f, static_cast<float>(kIntervals[intervalInput]));
        }

        for (int32_t j = 0; j < numSamples; j++) {
            auto offset = j * 6;
            *out1++ = in[offset + 5]; // unixtime
            *out2++ = in[offset + 0]; // open
            *out3++ = in[offset + 1]; // close
            *out4++ = in[offset + 2]; // high
            *out5++ = in[offset + 3]; // low
            *out6++ = in[offset + 4]; // volume
        }

        mReadSamples += numSamples;

        if (mReadSamples >= mWrittenSamples) {
            mInputHasChanged = false;
        }
    }

    /*********************************************************************/
    void GraphDataPlaceTrade::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

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

        for (int32_t i = 0; i < numSamples; i++) {
            auto time = *unixtimeInput++;
            auto decision = *decisionInput++;
            auto conviction = *convictionInput++;

            *unixtimeOut++ = time;
            *decisionOut++ = decision;
            *convictionOut++ = conviction;
        }
    }

    /*********************************************************************/
    void GraphDataBuffer::Reset() {
        mReadSamples = 0;
        mNode->ForEachInput(
            [&](NodeGraphInput& input) {
                if (input.HasInputNode() && input.GetInputNode()->IsOutOfDate()) {
                    mInputHasChanged = true;
                    mWrittenSamples = 0;
                }
            });
    }

    void GraphDataBuffer::Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        if (numSamples > numCacheSamples) {
            numCacheSamples = numSamples;
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

        if (mReadSamples < mWrittenSamples) {
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

        if (mReadSamples >= mWrittenSamples) {
            mInputHasChanged = false;
        }
    }

}
