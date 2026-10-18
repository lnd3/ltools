#include "nodegraph/operations/NodeGraphOpSource.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    /*********************************************************************/
    void GraphSourceConstants::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input0 = inputs.at(0).GetIterator(1);
        auto input1 = inputs.at(1).GetIterator(1);
        auto input2 = inputs.at(2).GetIterator(1);
        auto input3 = inputs.at(3).GetIterator(1);
        auto output0 = outputs.at(0).GetIterator(1);
        auto output1 = outputs.at(1).GetIterator(1);
        auto output2 = outputs.at(2).GetIterator(1);
        auto output3 = outputs.at(3).GetIterator(1);

        for (int8_t i = 0; i < mNumOutputs; i++) {
            *output0 = *input0;
            *output1 = *input1;
            *output2 = *input2;
            *output3 = *input3;
        }
    }

    void GraphSourceConstants::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    /*********************************************************************/
    void GraphSourceTime::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        auto input0 = inputs.at(0).GetIterator(numSamples);
        auto input1 = inputs.at(1).GetIterator(numSamples);
        auto input2 = inputs.at(2).GetIterator(numSamples);
        auto input3 = inputs.at(3).GetIterator(numSamples);
        auto output0 = outputs.at(0).GetIterator(numSamples);
        auto output1 = outputs.at(1).GetIterator(numSamples);
        auto output2 = outputs.at(2).GetIterator(numSamples);
        auto output3 = outputs.at(3).GetIterator(numSamples);

        if (inputs.at(4).Get() > 0.5f) {
            mAudioTick = 0;
            mFrameTick = 0;

            for (int32_t i = 0; i < numSamples; i++) {
                mAudioTick++;

                *input0 = static_cast<float>(mAudioTick);
                *input1 = static_cast<float>(mFrameTick);
                *input2 = mAudioTick / static_cast<float>(mAudioRate);
                *input3 = mFrameTick / static_cast<float>(mFrameRate);

                *output0++ = 0.0f;
                *output1++ = 0.0f;
                *output2++ = 0.0f;
                *output3++ = 0.0f;
            }
        }
        else {
            for (int32_t i = 0; i < numSamples; i++) {
                mAudioTick++;

                *input0 = static_cast<float>(mAudioTick);
                *input1 = static_cast<float>(mFrameTick);
                *input2 = mAudioTick / static_cast<float>(mAudioRate);
                *input3 = mFrameTick / static_cast<float>(mFrameRate);

                *output0++ = *input0++;
                *output1++ = *input1++;
                *output2++ = *input2++;
                *output3++ = *input3++;
            }
        }
    }

    void GraphSourceTime::Tick(int32_t, float deltaTime) {
        mFrameTick++;
        mFrameTime += deltaTime;
    }

    /*********************************************************************/
    void GraphSourceText::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto textIn = inputs.at(0).GetText(16);
        outputs.at(0).SetText(textIn);
    }

    /*********************************************************************/
    void GraphSourceConstants2::Process(int32_t, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input0 = &inputs.at(0).Get();
        auto input1 = &inputs.at(1).Get();
        auto input2 = &inputs.at(2).Get();
        auto input3 = &inputs.at(3).Get();
        auto min = inputs.at(4).Get();
        auto max = inputs.at(5).Get();
        auto output0 = outputs.at(0).GetIterator(1);
        auto output1 = outputs.at(1).GetIterator(1);
        auto output2 = outputs.at(2).GetIterator(1);
        auto output3 = outputs.at(3).GetIterator(1);

        for (int8_t i = 0; i < mNumOutputs; i++) {
            *input0 = l::math::clamp(*input0, min, max);
            *input1 = l::math::clamp(*input1, min, max);
            *input2 = l::math::clamp(*input2, min, max);
            *input3 = l::math::clamp(*input3, min, max);
            *output0 = *input0;
            *output1 = *input1;
            *output2 = *input2;
            *output3 = *input3;
        }
    }
}
