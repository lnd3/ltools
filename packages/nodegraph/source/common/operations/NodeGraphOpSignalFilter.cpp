#include "nodegraph/operations/NodeGraphOpSignalFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Stateful filtering operations */
        /*********************************************************************/
    void SignalFilterBase::DefaultDataInit() {
		NodeGraphOp::DefaultDataInit();

        mSync = 0.0f;
        mSamplesUntilUpdate = 0.0f;

        Reset();
    }

    void SignalFilterBase::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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
    void SignalFilterLowpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    float SignalFilterLowpass::ProcessSignal(float input, float cutoff, float resonance) {
        float inputValueInbetween = (mInputValuePrev + input) * 0.5f;
        for (int32_t oversample = 0; oversample < 2; oversample++) {
            mState0 = resonance * mState0 - cutoff * (mState1 + (oversample == 0 ? inputValueInbetween : input));
            mState1 = resonance * mState1 + cutoff * mState0;
        }
        mInputValuePrev = input;
        return -mState1;
    }

    /*********************************************************************/
    void SignalFilterHighpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    float SignalFilterHighpass::ProcessSignal(float input, float cutoff, float resonance) {
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
    void SignalFilterChamberlain2pole::DefaultDataInit() {
        SignalFilterBase::DefaultDataInit();

        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
    }

    void SignalFilterChamberlain2pole::Reset() {
        for (int32_t i = 0; i < 4; i++) {
            mState.at(i) = 0.0f;
        }
    }

    void SignalFilterChamberlain2pole::UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {
        mMode = static_cast<int32_t>(3.0f * mInputManager.GetValueNext(mNumDefaultInputs + 0) + 0.5f);
        mScale = l::math::sqrt(mInputManager.GetValueNext(3));
        mScaleFilter.SetConvergenceFactor().SetTarget(mScale).SnapAt();
    }

    float SignalFilterChamberlain2pole::ProcessSignal(float input, float cutoff, float resonance) {
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
    void SignalFilterMovingAverage::DefaultDataInit() {
        NodeGraphOp::DefaultDataInit();

        mSync = 0.0f;
        mSamplesUntilUpdate = 0.0f;

        Reset();
    }

    void SignalFilterMovingAverage::Reset() {
        if (mFilterState.size() != mDefaultKernelSize) {
            mFilterState.resize(mDefaultKernelSize);
            mFilterWeight.resize(mDefaultKernelSize);
        }
        mFilterStateIndex = 0;
        mFilterInit = true;
    }

    void SignalFilterMovingAverage::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);
        mSync = inputs.at(0).Get();
        if (mSync > 0.5f) {
            mNode->SetInput(1, 0.0f, numSamples);
        }

        auto output = outputs.at(0).GetIterator(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mInputManager.NodeUpdate(inputs, mUpdateRate);
                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                float width = l::math::max2(mInputManager.GetValueNext(3), 1.0f);
                float balance = mInputManager.GetValueNext(4);
                float weightAccent = mInputManager.GetValueNext(5);

                int32_t widthInt = 1 + static_cast<int32_t>(width);
                int32_t bufferSize = widthInt;
                float widthFrac = width - l::math::floor(width);

                auto hasWeights = inputs.at(2).HasInputNode();

                for (int32_t i = start; i < end; i++) {
                    float inputValue = mInputManager.GetValueNext(1);
                    float inputWeight = hasWeights ? mInputManager.GetValueNext(2) : 1.0f;

                    if (mFilterInit || mWidth != widthInt) {
                        mWidth = widthInt;
                        mFilterInit = false;
                        mFilterStateIndex = 0;

                        if (mFilterState.size() < bufferSize) {
                            mFilterState.resize(bufferSize);
                            mFilterWeight.resize(bufferSize);
                        }
                        for (int32_t j = 0; j < widthInt; j++) {
                            mFilterState[j] = inputValue;
                            mFilterWeight[j] = inputWeight;
                        }
                    }

                    mFilterState[mFilterStateIndex] = inputValue;
                    mFilterWeight[mFilterStateIndex] = l::math::pow(inputWeight, weightAccent);
                    mFilterStateIndex = (mFilterStateIndex + 1) % bufferSize; // buffer is 1 larger than the truncated filter size so we can smooth on the last one

                    float outVal = 0.0;
                    float balanceFactor = 1.0f - balance;
                    float balanceDelta = balance / width;
                    float balanceDivisorSum = 0.0f;
                    { // remove a part of the first sample of the sum as it is not part of the moving average
                        outVal += mFilterWeight[mFilterStateIndex] * mFilterState[mFilterStateIndex] * balanceFactor * widthFrac;
                        balanceDivisorSum += mFilterWeight[mFilterStateIndex] * balanceFactor * widthFrac;
                        balanceFactor += balanceDelta * widthFrac;
                    }
                    for (int32_t j = mFilterStateIndex + 1; j < bufferSize; j++) {
                        outVal += mFilterWeight[j] * mFilterState[j] * balanceFactor;
                        balanceDivisorSum += mFilterWeight[j] * balanceFactor;
                        balanceFactor += balanceDelta;
                    }
                    for (int32_t j = 0; j < mFilterStateIndex; j++) {
                        outVal += mFilterWeight[j] * mFilterState[j] * balanceFactor;
                        balanceDivisorSum += mFilterWeight[j] * balanceFactor;
                        balanceFactor += balanceDelta;
                    }

                    auto signal = outVal / balanceDivisorSum;

                    *output++ = signal;
                }
            }
        );
    }

}
