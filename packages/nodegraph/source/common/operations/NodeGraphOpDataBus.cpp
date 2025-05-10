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
        mInputReadPos = 0;
        mInputWritePos = 0;
    }

    void GraphDataCandleStickDataIn::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto in = &inputs.at(0).Get(numSamples * 6, mInputReadPos);
        float* outInterleaved = &outputs.at(0).Get(numSamples * 6);
        memcpy(outInterleaved, in, static_cast<size_t>(sizeof(float) * numSamples * 6));

        float* out0 = &outputs.at(1).Get(numSamples);
        float* out1 = &outputs.at(2).Get(numSamples);
        float* out2 = &outputs.at(3).Get(numSamples);
        float* out3 = &outputs.at(4).Get(numSamples);
        float* out4 = &outputs.at(5).Get(numSamples);
        float* out5 = &outputs.at(6).Get(numSamples);

        for (int32_t j = 0; j < numSamples; j++) {
            *out0++ = in[6 * j + 0];
            *out1++ = in[6 * j + 1];
            *out2++ = in[6 * j + 2];
            *out3++ = in[6 * j + 3];
            *out4++ = in[6 * j + 4];
            *out5++ = in[6 * j + 5];
        }

        mInputReadPos += numSamples * 6;
    }

    /*********************************************************************/
    void GraphDataTradeSignal::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto in = &inputs.at(0).Get(numSamples);
        auto out = &outputs.at(0).Get(numSamples);
        memcpy(out, in, static_cast<size_t>(sizeof(float) * numSamples));
    }

    /*********************************************************************/
    void GraphDataInputBuffer::Reset() {
        mInputReadPos = 0;
        mInputWritePos = 0;
    }

    void GraphDataInputBuffer::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto& inInterleaved = inputs.at(0);
        float* outInterleaved = &outputs.at(0).Get(numSamples * 6);
        memcpy(outInterleaved, &inInterleaved.Get(numSamples, mInputReadPos), static_cast<size_t>(sizeof(float) * numSamples * 6));
    }

}
