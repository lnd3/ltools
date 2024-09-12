#include "nodegraph/operations/NodeGraphOpFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Stateful filtering operations */

    /*********************************************************************/
    void GraphFilterLowpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
        mNode->SetInput(1, 0.99f);
        mNode->SetInput(2, 0.01f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterLowpass::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float inputValue = inputs.at(0).Get();
        float cutoff = inputs.at(1).Get();
        float resonance = 1.0f - inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = 1.0f - resonance * cutoff;

        mState0 = rc * mState0 - cutoff * (mState1 + inputValue);
        mState1 = rc * mState1 + cutoff * mState0;

        outputs.at(0).mOutput = -mState1;
    }

    /*********************************************************************/
    void GraphFilterHighpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
        mNode->SetInput(1, 0.99f);
        mNode->SetInput(2, 0.01f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterHighpass::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float inputValue = inputs.at(0).Get();
        float cutoff = inputs.at(1).Get();
        float resonance = 1.0f - inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = 1.0f - resonance * cutoff;

        float v01 = mState0 - mState1;
        mState0 += cutoff * (inputValue - mState0 + rc * v01);
        mState1 += cutoff * v01;

        outputs.at(0).mOutput = inputValue - mState1;
    }

    /*********************************************************************/
    void GraphFilterChamberlain2pole::Reset() {
        for (int32_t i = 0; i < 4; i++) {
            mState.at(i) = 0.0f;
        }

        mSamplesUntilUpdate = 0.0f;
        mUpdateSamples = 4.0f;

        mNode->SetInput(1, 0.99f);
        mNode->SetInput(2, 0.01f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterChamberlain2pole::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = &inputs.at(0).Get(numSamples);

        auto mode = static_cast<int32_t>(3.0f * inputs.at(3).Get() + 0.5f);
        auto output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateSamples, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mCutoff = inputs.at(1).Get();
                mResonance = 1.0f - inputs.at(2).Get();

                mFreq = l::math::functions::sin(l::math::constants::PI_f * mCutoff * mCutoff / 2.0f);
                mScale = l::math::functions::sqrt(mResonance);
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float inputValue = *input++;
                    float inputValueInbetween = (mInputValuePrev + inputValue) * 0.5f;
                    for (int32_t oversample = 0; oversample < 2; oversample++) {
                        mState.at(0) = mState.at(0) + mFreq * mState.at(2);
                        mState.at(1) = mScale * (oversample == 0 ? inputValueInbetween : inputValue) - mState.at(0) - mResonance * mState.at(2);
                        mState.at(2) = mFreq * mState.at(1) + mState.at(2);
                        mState.at(3) = mState.at(1) + mState.at(0);
                    }
                    *output0++ = mState.at(mode);
                    mInputValuePrev = inputValue;
                }
            }
        );
    }
}
