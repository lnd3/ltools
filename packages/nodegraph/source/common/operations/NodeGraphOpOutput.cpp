#include "nodegraph/operations/NodeGraphOpOutput.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphOutputDebug::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        float value = inputs.at(0).Get();
        float friction = inputs.at(1).Get();
        mValue += friction * friction * (value - mValue);
        inputs.at(2).mInput.mInputFloatConstant = mValue;
    }

    /*********************************************************************/
    void GraphOutputSpeaker::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        auto& buffer = mAudioStream->GetWriteBuffer();

        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        float volume = inputs.at(2).Get();
        mFilterEnvelope.SetTarget(volume * volume).SnapAt();

        float attackMs = 40.0f;
        float releaseMs = 40.0f;
        float attack = l::math::functions::pow(0.01f, 1.0f / (attackMs * 44100.0f * 0.001f));
        float release = l::math::functions::pow(0.01f, 1.0f / (releaseMs * 44100.0f * 0.001f));

        float limit = mFilterEnvelope.Next();
        float limitedVolume = 0.95f * limit;

        float inVal0 = in0 * limitedVolume;
        float inVal1 = in1 * limitedVolume;
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

        mCurrentStereoPosition %= mAudioStream->GetPartTotalSize();
    }

    /*********************************************************************/
    void GraphOutputPlot::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* value = &inputs.at(0).Get(numSamples);
        int32_t outputSize = outputs.at(0).GetSize();
        float* output = &outputs.at(0).Get(outputSize);

        for (int32_t i = 0; i < numSamples; i++) {
            output[mCurIndex] = *value++;
            mCurIndex = (mCurIndex + 1) % outputSize;
        }
    }

}
