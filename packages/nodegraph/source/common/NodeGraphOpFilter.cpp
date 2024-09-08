#include "nodegraph/NodeGraphOpFilter.h"

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
    void GraphFilterEnvelope::Reset() {
        mEnvelope = 0.0f;
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 50.0f);
        mNode->SetInput(3, 50.0f);
        mNode->SetInput(4, 0.1f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_CUSTOM, 1.0f, 100000.0f);
        mNode->SetInputBound(3, InputBound::INPUT_CUSTOM, 1.0f, 100000.0f);
        mNode->SetInputBound(4, InputBound::INPUT_CUSTOM, 0.0001f, 1.0f);
    }

    void GraphFilterEnvelope::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float freqTarget = inputs.at(0).Get();
        float velocity = l::math::functions::pow(inputs.at(1).Get(), 0.5f);
        int32_t attackFrames = static_cast<int32_t>(inputs.at(2).Get() * 44100.0f / 1000.0f);
        int32_t releaseFrames = static_cast<int32_t>(inputs.at(3).Get() * 44100.0f / 1000.0f);
        float freqFade = inputs.at(4).Get();
        
        float attackFade = 1.0f - l::math::functions::pow(0.001f, 1.0f / (inputs.at(2).Get() * 44100.0f * 0.001f));
        float releaseFade = 1.0f - l::math::functions::pow(0.001f, 1.0f / (inputs.at(3).Get() * 44100.0f * 0.001f));

        if (freqTarget == 0.0f) {
            // trigger release
            if (mFrameCount > 0 && mFrameCount < attackFrames + 1) {
                mFrameCount = attackFrames + 2;
            }
            else if (mFrameCount > attackFrames + 1 + releaseFrames) {
                mFrameCount = 0;
            }
        }
        else {
            if (mFrameCount == 0) {
                mFreq = freqTarget;
            }
            else {
                mFrameCount = attackFrames + 2;
                mEnvelopeTarget = velocity;
            }
        }

        if (freqTarget != 0 && mFrameCount < attackFrames) {
            // attack
            mEnvelopeTarget = velocity;
            mFrameCount++;
        }
        else if (freqTarget != 0 && mFrameCount == attackFrames + 1){
            // sustain
        }
        else if (freqTarget == 0 && mFrameCount > attackFrames + 1) {
            // release
            mEnvelopeTarget = 0.0f;
            mFrameCount++;
            if (mFrameCount > attackFrames + 1 + releaseFrames) {
                mFrameCount = 0;
            }
        }

        float delta = mEnvelopeTarget - mEnvelope;
        if (delta > 0) {
            mEnvelope += attackFade * delta;
        }
        else {
            mEnvelope += releaseFade * delta;
        }

        if (freqTarget != 0.0f) {
            // note on
            mFreq += freqFade * freqFade * (freqTarget - mFreq);
        }
        else {
            // note off
        }

        outputs.at(0).mOutput = mFreq;
        outputs.at(1).mOutput = l::math::functions::pow(mEnvelope, 0.5f);
    }

}
