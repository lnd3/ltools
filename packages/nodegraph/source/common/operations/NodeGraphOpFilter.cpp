#include "nodegraph/operations/NodeGraphOpFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Stateful filtering operations */
        /*********************************************************************/
    void GraphFilterBase::Reset() {
        mReset = 0.0f;
        mSamplesUntilUpdate = 0.0f;

        mCutoff = 0.0f;
        mResonance = 0.0f;
        mCutoffCPS = 0.0f;

        mNode->SetInput(0, 0.0f);
        mNode->SetInput(2, 1.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);

        ResetInput();
        ResetSignal();
    }

    void GraphFilterBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = &inputs.at(1).Get(numSamples);
        auto output = &outputs.at(0).Get(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateSamples, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mReset = inputs.at(0).Get();
                if (mReset > 0.5f) {
                    mNode->SetInput(1, 0.0f, numSamples);
                }

                mCutoff = inputs.at(2).Get();
                mCutoffCPS = l::math::functions::sin(l::math::constants::PI_f * mCutoff * mCutoff * 0.5f);
                mResonance = 1.0f - inputs.at(3).Get();

                mCutoffFilter.SetConvergence().SetTarget(mCutoffCPS).SnapAt();
                mResonanceFilter.SetConvergence().SetTarget(mResonance).SnapAt();

                UpdateSignal(inputs, outputs);
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float inputValue = *input++;
                    float signal = ProcessSignal(inputValue, mCutoffFilter.Next(), mResonanceFilter.Next());
                    *output++ = signal;
                }
            }
        );
    }
    /*********************************************************************/
    void GraphFilterLowpass::ResetInput() {
    }

    void GraphFilterLowpass::ResetSignal() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    float GraphFilterLowpass::ProcessSignal(float input, float cutoff, float resonance) {
        float inputValueInbetween = (mInputValuePrev + input) * 0.5f;
        for (int32_t oversample = 0; oversample < 2; oversample++) {
            mState0 = resonance * mState0 - cutoff * (mState1 + (oversample == 0 ? inputValueInbetween : input));
            mState1 = resonance * mState1 + cutoff * mState0;
        }
        mInputValuePrev = input;
        return -mState1;
    }

    /*********************************************************************/
    void GraphFilterHighpass::ResetInput() {
    }

    void GraphFilterHighpass::ResetSignal() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    float GraphFilterHighpass::ProcessSignal(float input, float cutoff, float resonance) {
        float inputValueInbetween = (mInputValuePrev + input) * 0.5f;
        for (int32_t oversample = 0; oversample < 2; oversample++) {
            float v01 = mState0 - mState1;
            mState0 += cutoff * ((oversample == 0 ? inputValueInbetween : input) - mState0 + resonance * v01);
            mState1 += cutoff * v01;
        }
        mInputValuePrev = input;
        return input - mState1;
    }

    /*********************************************************************/
    void GraphFilterChamberlain2pole::ResetInput() {
        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterChamberlain2pole::ResetSignal() {
        for (int32_t i = 0; i < 4; i++) {
            mState.at(i) = 0.0f;
        }
    }

    void GraphFilterChamberlain2pole::UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        mMode = static_cast<int32_t>(3.0f * inputs.at(mNumDefaultInputs + 0).Get() + 0.5f);
        mScale = l::math::functions::sqrt(mResonance);
        mScaleFilter.SetConvergence().SetTarget(mScale).SnapAt();
    }

    float GraphFilterChamberlain2pole::ProcessSignal(float input, float cutoff, float resonance) {
        float inputValueInbetween = (mInputValuePrev + input) * 0.5f;
        float scale = mScaleFilter.Next();
        cutoff *= 0.5f;
        for (int32_t oversample = 0; oversample < 2; oversample++) {
            mState.at(0) = mState.at(0) + cutoff * mState.at(2);
            mState.at(1) = scale * (oversample == 0 ? inputValueInbetween : input) - mState.at(0) - resonance * mState.at(2);
            mState.at(2) = cutoff * mState.at(1) + mState.at(2);
            mState.at(3) = mState.at(1) + mState.at(0);
        }
        mInputValuePrev = input;

        return mState.at(mMode);
    }
}