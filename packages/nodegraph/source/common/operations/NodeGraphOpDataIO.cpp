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
    void GraphCache::Reset() {
        mReadSamples = 0;
        if (mNode->IsOutOfDate2()) {
            mInputHasChanged = true;
            mWrittenSamples = 0;
        }
    }

    void GraphCache::ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        if (static_cast<int32_t>(mBuffer.size()) != numCacheSamples * mChannels) {
            mBuffer.resize(static_cast<size_t>(numCacheSamples * mChannels));
        }

        float* input[4];
        for (int32_t j = 0; j < mChannels; j++) {
            input[j] = &inputs.at(j).Get(numSamples);
        }
        auto buf = mBuffer.data() + writtenSamples * mChannels;
        for (int32_t j = 0; j < numSamples; j++) {
            for (int32_t i = 0; i < mChannels; i++) {
                *buf++ = *(input[i])++;
            }
        }
    }

    void GraphCache::ProcessReadCached(int32_t readSamples, int32_t numSamples, int32_t, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>& outputs) {
        float* output[4];
        for (int32_t j = 0; j < mChannels; j++) {
            output[j] = &outputs.at(j).Get(numSamples);
        }

        auto buf = mBuffer.data() + readSamples * mChannels;

        for (int32_t j = 0; j < numSamples; j++) {
            for (int32_t i = 0; i < mChannels; i++) {
                *(output[i])++ = *buf++;
            }
        }
    }

}
