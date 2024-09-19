#include "nodegraph/operations/NodeGraphOpControl.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"
#include "math/MathSmooth.h"

#include <math.h>

namespace l::nodegraph {
    /*********************************************************************/

    void GraphControlBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mNodeInputManager.ProcessUpdate(inputs, numSamples, mUpdateRate);

        float sync = inputs.at(0).Get();
        mUpdateRate = mNodeInputManager.GetValue(1);
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        auto output0 = outputs.at(0).GetIterator(numSamples);
        auto output1 = outputs.at(1).GetIterator(numSamples);


        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNodeInputManager.NodeUpdate(inputs, mUpdateRate);
                mUpdateRate = mNodeInputManager.GetValue(1);

                UpdateSignal(mNodeInputManager);

                return mUpdateRate;
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
        if (mFreqTarget != 0.0f) {
            inputManager.SetTarget(mFreqId, mFreqTarget);
        }
        inputManager.SetDuration(mFreqId, 100.0f * inputManager.GetValue(3));
        mAttackFrames = l::audio::GetAudioTicksFromMS(inputManager.GetValue(4));
        mReleaseFrames = l::audio::GetAudioTicksFromMS(inputManager.GetValue(5));
        mAttackFactor = inputManager.GetValue(4);
        mReleaseFactor = inputManager.GetValue(5);
    }

    std::pair<float, float> GraphControlEnvelope::ProcessSignal(NodeInputManager& inputManager) {
        float velocity = inputManager.GetValueNext(2);
        float freq = 0.0f;

        bool noteOn = mFreqTarget != 0.0f;
        bool differentNote = mNoteOn && noteOn && mFreqTargetPrev != mFreqTarget;
        if (noteOn && !mNoteOn || differentNote) {
            mNoteOn = true;

            mFreqTargetPrev = mFreqTarget;

            mNodeInputManager.SetDuration(8, mAttackFactor);
            mNodeInputManager.SetTarget(8, l::math::functions::sqrt(velocity));

            if (mFrameCount == 0) {
                // if note was off we set freq immediately
                freq = mFreqTarget;
            }
            mFrameCount = 0;
        }
        else if (!noteOn && mNoteOn) {
            mNoteOn = false;

            mNodeInputManager.SetDuration(8, mReleaseFactor);
            mNodeInputManager.SetTarget(8, 0.0f);

            if (mFrameCount > 0 && mFrameCount < mAttackFrames + 1) {
                // still in attack so fast forward to release
                mFrameCount = mAttackFrames + 2;
            }
            else if (mFrameCount > mAttackFrames + 1 + mReleaseFrames) {
                // if released, reset
                mFrameCount = 0;
            }
        }

        if (mNoteOn) {
            if (mFrameCount < mAttackFrames) {
                // attack
                mFrameCount++;
            }
            else if (mFrameCount == mAttackFrames + 1) {
                // sustain, linger on frame after last attack frame
            }
        }
        else if (mFrameCount > 0) {
            // release
            mFrameCount++;
            if (mFrameCount > mAttackFrames + 1 + mReleaseFrames) {
                mFrameCount = 0;
            }
        }

        freq = inputManager.GetValueNext(mFreqId);

        float envelope = mNodeInputManager.GetValueNext(8);
        return { freq, envelope };
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

            mGainTarget = 3.0f*velocity;
            mFreqSmoothing = fade;
            mFreqSmoothing *= mFreqSmoothing * 0.5f;

            inputManager.SetDuration(8, attack, 0.01f);
            inputManager.SetDuration(9, attack + release, 0.01f);
            inputManager.SetValue(8, 0.0f);
            inputManager.SetValue(9, 1.0f);
        }
    }

    void GraphControlArpeggio::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mNodeInputManager.ProcessUpdate(inputs, numSamples, mUpdateRate);
        float sync = mNodeInputManager.GetValue(0);
        float bpm = mNodeInputManager.GetValue(1);
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
                mNodeInputManager.NodeUpdate(inputs, mUpdateRate);
                mUpdateRate = mNodeInputManager.GetValue(1);

                UpdateSignal(mNodeInputManager);

                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    mNodeInputManager.SetTarget(8, 1.0f);
                    mNodeInputManager.SetTarget(9, 0.0f);
                    float attack = mNodeInputManager.GetValueNext(8);
                    float release = mNodeInputManager.GetValueNext(9);
                    float gain = mGainTarget * (attack * release);
                    mGain += 0.25f * (gain - mGain);

                    mFreq += mFreqSmoothing * (mFreqTarget - mFreq);

                    *output0++ = mFreq;
                    *output1++ = mGain;
                }
            }
        );
    }

}
