#include "nodegraph/operations/NodeGraphOpControl.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"
#include "math/MathSmooth.h"

#include <math.h>

namespace l::nodegraph {
    /*********************************************************************/

    void GraphControlBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float sync = inputs.at(0).Get();
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
            mUpdateRate = mNodeInputManager.GetValue(1);
        }

        auto output0 = outputs.at(0).GetIterator(numSamples);
        auto output1 = outputs.at(1).GetIterator(numSamples);

        mNodeInputManager.ProcessUpdate(inputs, numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNodeInputManager.NodeUpdate(inputs);
                mUpdateRate = mNodeInputManager.GetValue(1);

                UpdateSignal(mNodeInputManager);
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    auto [out0, out1] = ProcessSignal(mNodeInputManager);
                    *output0++ = out0;
                    *output1++ = out1;
                }
            }
        );
    }

    /*********************************************************************/

    void GraphControlEnvelope::UpdateSignal(NodeInputManager& inputManager) {

        mFreqTarget = inputManager.GetValueNext(mFreqTargetId);
        inputManager.SetTarget(mFreqId, mFreqTarget);
        mAttackFrames = static_cast<int32_t>(inputManager.GetValue(4));
        mReleaseFrames = static_cast<int32_t>(inputManager.GetValue(5));
        mAttackFactor = l::audio::GetRWAFactorFromTicks(inputManager.GetValueNext(4), 0.01f);
        mReleaseFactor = l::audio::GetRWAFactorFromTicks(inputManager.GetValueNext(5), 0.01f);
    }

    std::pair<float, float> GraphControlEnvelope::ProcessSignal(NodeInputManager& inputManager) {
        float velocity = inputManager.GetValueNext(2);
        float freq = inputManager.GetValue(mFreqId);

        if (mFreqTarget == 0.0f) {
            // trigger release
            if (mFrameCount > 0 && mFrameCount < mAttackFrames + 1) {
                mFrameCount = mAttackFrames + 2;
            }
            else if (mFrameCount > mAttackFrames + 1 + mReleaseFrames) {
                mFrameCount = 0;
            }
        }
        else {
            if (mFrameCount == 0) {
                freq = mFreqTarget;
            }
            else if (mFreqTarget != 0) {
                mFrameCount = mAttackFrames + 2;
                mEnvelopeTarget = velocity;
            }
        }

        if (mFreqTarget != 0 && mFrameCount < mAttackFrames) {
            // attack
            mEnvelopeTarget = velocity;
            mFrameCount++;
        }
        else if (mFreqTarget != 0 && mFrameCount == mAttackFrames + 1) {
            // sustain
        }
        else if (mFreqTarget == 0 && mFrameCount > mAttackFrames + 1) {
            // release
            mEnvelopeTarget = 0.0f;
            mFrameCount++;
            if (mFrameCount > mAttackFrames + 1 + mReleaseFrames) {
                mFrameCount = 0;
            }
        }

        float delta = mEnvelopeTarget - mEnvelope;
        if (delta > 0) {
            mEnvelope += mAttackFactor * delta;
        }
        else {
            mEnvelope += mReleaseFactor * delta;
        }

        if (mFreqTarget != 0.0f) {
            // note on
            freq = inputManager.GetValueNext(mFreqId);
        }
        else {
            // note off
        }

        return { freq, mEnvelope };
    }

    /*********************************************************************/

    void GraphControlArpeggio::UpdateSignal(NodeInputManager& inputManager) {
        float attack = inputManager.GetValue(4);
        float release = inputManager.GetValue(5);
        float attackFactor = l::audio::GetRWAFactorFromMS(attack, 0.001f);
        float releaseFactor = l::audio::GetRWAFactorFromMS(release, 0.001f);
        mGainAttack = attackFactor;
        mGainRelease = releaseFactor;

        if (mNotes.empty()) {
            mGainTarget = 0.0f;
        }
        else {
            mNoteIndex = mNoteIndex % mNotes.size();
            mFreqTarget = l::audio::GetFrequencyFromNote(static_cast<float>(mNotes.at(mNoteIndex)));
            mNoteIndex++;

            auto velocity = inputManager.GetValue(2);
            auto fade = inputManager.GetValue(3);

            mGainTarget = velocity;
            mFreqSmoothing = fade;
            mFreqSmoothing *= mFreqSmoothing * 0.5f;
        }
    }

    void GraphControlArpeggio::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mNodeInputManager.ProcessUpdate(inputs, numSamples);

        float sync = mNodeInputManager.GetValue(0);
        float bpm = mNodeInputManager.GetValueNext(1);

        mUpdateRate = 44100.0f * 60.0f / (4.0f * bpm);
        
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        {
            auto noteIdsOn = mNodeInputManager.GetArray(6);
            auto noteIdsOff = mNodeInputManager.GetArray(7);

            if (!mNotes.empty()) {
                for (int32_t i = 0; i < gPolyphony; i++) {
                    if (l::math::functions::equal(*noteIdsOff, l::audio::gNoNote_f)) {
                        break;
                    }
                    int32_t noteOffId = static_cast<int32_t>(*noteIdsOff + 0.5f);
                    auto it = std::find(mNotes.begin(), mNotes.end(), noteOffId);
                    if (it != mNotes.end()) {
                        mNotes.erase(it);
                    }
                    noteIdsOff++;
                }
            }

            if (mNotes.empty()) {
                mGainTarget = 0.0f;
            }
            for (int32_t i = 0; i < gPolyphony; i++) {
                if (l::math::functions::equal(*noteIdsOn, l::audio::gNoNote_f)) {
                    break;
                }
                int32_t noteOnId = static_cast<int32_t>(*noteIdsOn + 0.5f);
                auto it = std::find(mNotes.begin(), mNotes.end(), noteOnId);
                if (it == mNotes.end()) {
                    mNotes.push_back(noteOnId);
                }
                noteIdsOn++;
            }
        }

        auto output0 = outputs.at(0).GetIterator(numSamples);
        auto output1 = outputs.at(1).GetIterator(numSamples);
        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNodeInputManager.NodeUpdate(inputs);
                mUpdateRate = mNodeInputManager.GetValue(1);

                UpdateSignal(mNodeInputManager);
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float delta = mGainTarget - mGain;
                    if (delta > 0) {
                        mGain += mGainAttack * l::math::smooth::smoothPolyh3(delta);
                    }
                    else {
                        mGain += mGainRelease * (-l::math::smooth::smoothPolyh3(-delta));
                    }
                    mFreq += mFreqSmoothing * (mFreqTarget - mFreq);

                    *output0++ = mFreq;
                    *output1++ = mGain;
                }
            }
        );
    }

}
