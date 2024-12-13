#include "nodegraph/operations/NodeGraphOpFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Stateful filtering operations */
        /*********************************************************************/
    void GraphFilterBase::DefaultDataInit() {
		NodeGraphOp::DefaultDataInit();

        mSync = 0.0f;
        mSamplesUntilUpdate = 0.0f;

        Reset();
    }

    void GraphFilterBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);
        mSync = inputs.at(0).Get();
        if (mSync > 0.5f) {
            mNode->SetInput(1, 0.0f, numSamples);
        }

        auto output = outputs.at(0).GetIterator(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mInputManager.NodeUpdate(inputs, mUpdateRate);

                UpdateSignal(inputs, outputs);

                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float inputValue = mInputManager.GetValueNext(1);
                    float signal = ProcessSignal(inputValue, mInputManager.GetValueNext(2), mInputManager.GetValueNext(3));
                    *output++ = signal;
                }
            }
        );
    }
    /*********************************************************************/
    void GraphFilterLowpass::Reset() {
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
    void GraphFilterHighpass::Reset() {
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
    void GraphFilterChamberlain2pole::DefaultDataInit() {
        GraphFilterBase::DefaultDataInit();

        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterChamberlain2pole::Reset() {
        for (int32_t i = 0; i < 4; i++) {
            mState.at(i) = 0.0f;
        }
    }

    void GraphFilterChamberlain2pole::UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {
        mMode = static_cast<int32_t>(3.0f * mInputManager.GetValueNext(mNumDefaultInputs + 0) + 0.5f);
        mScale = l::math::sqrt(mInputManager.GetValueNext(3));
        mScaleFilter.SetConvergenceFactor().SetTarget(mScale).SnapAt();
    }

    float GraphFilterChamberlain2pole::ProcessSignal(float input, float cutoff, float resonance) {
        float inputValueInbetween = (mInputValuePrev + input) * 0.5f;
        float scale = mScaleFilter.Next();
        resonance *= 0.99f * (cutoff * 0.15f + 0.85f); // adjust resonance slightly down at low frequencies to avoid oscillation
        cutoff *= 0.5f;
        resonance = (1.0f - resonance);
        for (int32_t oversample = 0; oversample < 2; oversample++) {
            mState.at(0) = mState.at(0) + cutoff * mState.at(2);
            mState.at(1) = scale * (oversample == 0 ? inputValueInbetween : input) - mState.at(0) - resonance * mState.at(2);
            mState.at(2) = cutoff * mState.at(1) + mState.at(2);
            mState.at(3) = mState.at(1) + mState.at(0);
        }
        mInputValuePrev = input;

        return mState.at(mMode);
    }

    /*********************************************************************/
    void GraphFilterMovingAverage::Reset() {
        mFilterState.resize(mKernelSize);
        mFilterStateIndex = 0;
        mFilterInit = true;
    }

    float GraphFilterMovingAverage::ProcessSignal(float input, float, float) {
        if (mUndefinedValue == input) {
            return input;
        }

        mFilterState[mFilterStateIndex] = input;
        float width = mInputManager.GetValueNext(mNumDefaultInputs + 0);
        int32_t widthInt = static_cast<int32_t>(width);
        mFilterStateIndex = (mFilterStateIndex + 1) % widthInt;

        if (widthInt == 1) {
            return input;
        }

        if (mFilterInit) {
            if (mFilterStateIndex + 1 < widthInt) {
                widthInt = mFilterStateIndex + 1;
            }
            else {
                mFilterInit = false;
            }
        }

        float outVal = 0.0;
        for (int32_t i = 0; i < widthInt; i++) {
            outVal += mFilterState[i];
        }
        return outVal / static_cast<float>(widthInt);
    }
}
