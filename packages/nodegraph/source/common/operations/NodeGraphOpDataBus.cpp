#include "nodegraph/operations/NodeGraphOpDataBus.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphDataBusDataIn::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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
    void GraphDataBusDataOut::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);

        float* output = &outputs.at(0).Get(mOutputDataStride * numSamples);
        for (int32_t i = 0; i < numSamples; i++) {
            for (int32_t j = 0; j < mOutputDataStride; j++) {
                output[mOutputDataStride * i + j] = mInputManager.GetValueNext(j);
            }
        }
    }

    /*********************************************************************/
    void GraphDataCandleStickDataIn::Reset() {
        mReadSamples = 0;
    }

    void GraphDataCandleStickDataIn::InputHasChanged(int32_t numSamplesWritten) {
        mInputHasChanged = true;
        mWrittenSamples = numSamplesWritten;
    }

    int32_t GraphDataCandleStickDataIn::GetNumSamplesLeft() {
        return mWrittenSamples - mReadSamples;
    }

    void GraphDataCandleStickDataIn::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto in = &inputs.at(0).Get(numSamples * 6 + mReadSamples * 6, mReadSamples * 6);

        float* out0 = &outputs.at(0).Get(numSamples);
        float* out1 = &outputs.at(1).Get(numSamples);
        float* out2 = &outputs.at(2).Get(numSamples);
        float* out3 = &outputs.at(3).Get(numSamples);
        float* out4 = &outputs.at(4).Get(numSamples);
        float* out5 = &outputs.at(5).Get(numSamples);

        for (int32_t j = 0; j < numSamples; j++) {
            auto offset = j * 6;
            *out0++ = in[offset + 0];
            *out1++ = in[offset + 1];
            *out2++ = in[offset + 2];
            *out3++ = in[offset + 3];
            *out4++ = in[offset + 4];
            *out5++ = in[offset + 5];
        }

        mReadSamples += numSamples;

        if (mReadSamples >= mWrittenSamples) {
            mInputHasChanged = false;
        }
    }

    /*********************************************************************/
    void GraphDataTradeSignal::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto in = &inputs.at(0).Get(numSamples);
        auto out = &outputs.at(0).Get(numSamples);
        memcpy(out, in, static_cast<size_t>(sizeof(float) * numSamples));
    }

    /*********************************************************************/
    void GraphDataBuffer::Reset() {
        mReadSamples = 0;
    }

    void GraphDataBuffer::InputHasChanged(int32_t numSamplesWritten) {
        mInputHasChanged = true;
        mWrittenSamples = numSamplesWritten;
    }

    void GraphDataBuffer::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        if (mInputHasChanged) {
            if (mBuffer.size() < mWrittenSamples * mChannels) {
                mBuffer.resize(mWrittenSamples * mChannels);
            }

            float* input[4];
            for (int32_t j = 0; j < mChannels; j++) {
                input[j] = &inputs.at(j).Get(numSamples);
            }
            auto buf = mBuffer.data();
            for (int32_t j = 0; j < numSamples; j++) {
                for (int32_t i = 0; i < mChannels; j++) {
                    *buf++ = *(input[i])++;
                }
            }
        }

        float* output[4];
        for (int32_t j = 0; j < mChannels; j++) {
            output[j] = &outputs.at(j).Get(numSamples);
        }

        auto buf = mBuffer.data();

        for (int32_t j = 0; j < numSamples; j++) {
            for (int32_t i = 0; i < mChannels; j++) {
                *(output[i])++ = *buf++;
            }
        }

        mReadSamples += numSamples;

        if (mReadSamples >= mWrittenSamples) {
            mInputHasChanged = false;
        }
    }

}
