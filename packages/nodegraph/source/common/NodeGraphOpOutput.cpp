#include "nodegraph/NodeGraphOpOutput.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphOutputDebug::Reset() {
        mValue = 0.0;
        mNode->SetInput(1, 0.5f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
    }

    void GraphOutputDebug::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        float value = inputs.at(0).Get();
        float friction = inputs.at(1).Get();
        mValue += friction * friction * (value - mValue);
        inputs.at(2).mInput.mInputFloatConstant = mValue;
    }

    /*********************************************************************/
    void GraphOutputSpeaker::Reset() {
        mEnvelope = 0.0f;
        mNode->SetInput(2, 0.5f);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
    }

    void GraphOutputSpeaker::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        auto& buffer = mAudioStream->GetWriteBuffer();

        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        float volume = inputs.at(2).Get();
        volume *= volume;

        if (false) { // limiter, causes distorsion
            float limit = volume;
            volume *= 0.95f;

            float inPreamp0 = in0 * volume;
            float inPreamp1 = in1 * volume;

            if (inPreamp0 >= limit || inPreamp0 <= -limit) {
                if (inPreamp0 > 0.0f) {
                    inPreamp0 = limit + (1.0f - limit) * l::math::functions::sigmoidFast((inPreamp0 - limit) / ((1.0f - limit) * 1.5f));
                }
                else {
                    inPreamp0 = -(limit + (1.0f - limit) * l::math::functions::sigmoidFast((-inPreamp0 - limit) / ((1.0f - limit) * 1.5f)));
                }
            }
            if (inPreamp1 >= limit || inPreamp1 <= -limit) {
                if (inPreamp1 > 0.0f) {
                    inPreamp1 = limit + (1.0f - limit) * l::math::functions::sigmoidFast((inPreamp1 - limit) / ((1.0f - limit) * 1.5f));
                }
                else {
                    inPreamp1 = -(limit + (1.0f - limit) * l::math::functions::sigmoidFast((-inPreamp1 - limit) / ((1.0f - limit) * 1.5f)));
                }
            }
            buffer[mCurrentStereoPosition++] = inPreamp0;
            buffer[mCurrentStereoPosition++] = inPreamp1;
        }
        else {
            float attackMs = 40.0f;
            float releaseMs = 40.0f;
            float attack = l::math::functions::pow(0.01f, 1.0f / (attackMs * 44100.0f * 0.001f));
            float release = l::math::functions::pow(0.01f, 1.0f / (releaseMs * 44100.0f * 0.001f));

            float limit = volume;
            volume *= 0.95f;

            float inVal0 = in0 * volume;
            float inVal1 = in1 * volume;
            float inVal = inVal0 > inVal1 ? inVal0 : inVal1;
            if (inVal > mEnvelope) {
                mEnvelope = attack * (mEnvelope - inVal) + inVal;
            }
            else {
                mEnvelope = release * (mEnvelope - inVal) + inVal;
            }

            float out0 = 0.0f;
            float out1 = 0.0f;
            float envelopeAbs = l::math::functions::abs(mEnvelope);
            if (envelopeAbs > limit) {
                if (envelopeAbs > 1.0f) {
                    out0 = inVal0 / mEnvelope;
                    out1 = inVal1 / mEnvelope;
                }
                else {
                    out0 = inVal0 / (1.0f + mEnvelope - limit);
                    out1 = inVal1 / (1.0f + mEnvelope - limit);
                }
            }
            else {
                out0 = inVal0;
                out1 = inVal1;
            }

            buffer[mCurrentStereoPosition++] = out0;
            buffer[mCurrentStereoPosition++] = out1;
        }

        mCurrentStereoPosition %= mAudioStream->GetPartTotalSize();
    }

    /*********************************************************************/
    void GraphOutputPlot::Reset() {
        mNode->SetInput(0, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_NEG_1_POS_1);
    }

    void GraphOutputPlot::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* value = &inputs.at(0).Get(numSamples);
        int32_t outputSize = outputs.at(0).GetOutputSize();
        float* output = &outputs.at(0).GetOutput(outputSize);

        for (int32_t i = 0; i < numSamples; i++) {
            output[mCurIndex] = *value++;
            mCurIndex = (mCurIndex + 1) % outputSize;
        }
    }

}
