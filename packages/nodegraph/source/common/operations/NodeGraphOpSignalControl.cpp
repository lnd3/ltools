#include "nodegraph/operations/NodeGraphOpSignalControl.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"
#include "math/MathSmooth.h"

#include <math.h>

namespace l::nodegraph {
    /*********************************************************************/

    void SignalControlBase::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);

        float sync = mInputManager.GetValueNext(0);
        mUpdateRate = mInputManager.GetValueNext(1);
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        auto output0 = outputs.at(0).GetIterator(numSamples, 16.0f);
        auto output1 = outputs.at(1).GetIterator(numSamples, 16.0f);


        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mInputManager.NodeUpdate(inputs, mUpdateRate);
                mUpdateRate = mInputManager.GetValue(1);

                UpdateSignal(mInputManager);

                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    auto [out0, out1] = ProcessSignal(mInputManager);
                    *output0++ = out0;
                    *output1++ = out1;
                }
            }
        );
    }

    /*********************************************************************/

    void SignalControlEnvelope::UpdateSignal(InputManager&) {

        mFreqTarget = mInputManager.GetValueNext(6);
        if (mFreqTarget != 0.0f) {
            mInputManager.SetTarget(100, mFreqTarget);
        }
        mInputManager.SetDuration(100, 1000.0f * mInputManager.GetValueNext(3));
        mAttackFrames = l::audio::GetAudioTicksFromMS(mInputManager.GetValueNext(4));
        mReleaseFrames = l::audio::GetAudioTicksFromMS(mInputManager.GetValueNext(5));
        mAttackFactor = mInputManager.GetValueNext(4);
        mReleaseFactor = mInputManager.GetValueNext(5);

        mInputManager.SetUpdateRate(100, 1.0f);
        mInputManager.SetUpdateRate(101, 1.0f);
    }

    std::pair<float, float> SignalControlEnvelope::ProcessSignal(InputManager&) {
        float velocity = mInputManager.GetValueNext(2);
        float freq = 0.0f;

        bool noteOn = mFreqTarget != 0.0f;
        bool differentNote = mNoteOn && noteOn && mFreqTargetPrev != mFreqTarget;
        if ((noteOn && !mNoteOn) || differentNote) {
            mNoteOn = true;

            mFreqTargetPrev = mFreqTarget;

            mInputManager.SetDuration(101, mAttackFactor);
            mInputManager.SetTarget(101, l::math::sqrt(velocity));

            if (mFrameCount == 0) {
                // if note was off we set freq immediately
                freq = mFreqTarget;
            }
            mFrameCount = 0;
        }
        else if (!noteOn && mNoteOn) {
            mNoteOn = false;

            mInputManager.SetDuration(101, mReleaseFactor);
            mInputManager.SetTarget(101, 0.0f);

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

        freq = mInputManager.GetValueNext(100);

        float envelope = mInputManager.GetValueNext(101);
        return { freq, envelope };
    }

    /*********************************************************************/

    void SignalControlArpeggio::UpdateSignal(InputManager&) {
        float attack = mInputManager.GetValue(4);
        float release = mInputManager.GetValue(5);
        float attackFactor = l::audio::GetRWAFactorFromMS(attack, 0.001f, gArpeggioUpdateRate);
        float releaseFactor = l::audio::GetRWAFactorFromMS(release, 0.001f, gArpeggioUpdateRate);
        mGainAttack = attackFactor;
        mGainRelease = releaseFactor;

        if (mNotes.empty()) {
            mGainTarget = 0.0f;
            mInputManager.SetDuration(100, release, 0.01f);
            mInputManager.SetDuration(101, release, 0.01f);
            mInputManager.SetTarget(100, 0.0f);
            mInputManager.SetTarget(101, 1.0f);
        }
        else {
            if (mUpdateCounter % 2 == 0) {
                mNoteIndex = mNoteIndex % mNotes.size();
                mFreqTarget = l::audio::GetFrequencyFromNote(static_cast<float>(mNotes.at(mNoteIndex)));
                mNoteIndex++;

                auto velocity = mInputManager.GetValueNext(2);
                auto fade = mInputManager.GetValueNext(3);

                mGainTarget = 2.0f * velocity;
                mFreqSmoothing = fade;
                mFreqSmoothing *= mFreqSmoothing * 0.5f;

                mInputManager.SetDuration(100, attack, 0.01f);
                mInputManager.SetDuration(101, attack + release, 0.01f);
                mInputManager.SetTarget(100, 1.0f);
                mInputManager.SetTarget(101, 0.0f);
                //inputManager.SetValue(100, 0.0f);
                //inputManager.SetValue(101, 1.0f);

                mUpdateCounter = 0;
            }
            else {
                mInputManager.SetDuration(100, release, 0.01f);
                mInputManager.SetDuration(101, release, 0.01f);
                mInputManager.SetTarget(100, 0.0f);
                mInputManager.SetTarget(101, 1.0f);
            }
            mUpdateCounter++;
        }
    }

    void SignalControlArpeggio::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);
        float sync = mInputManager.GetValueNext(0);
        float bpm = mInputManager.GetValueNext(1);
        mUpdateRate = 44100.0f * 60.0f / (2.0f * 4.0f * bpm);
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        {
            auto noteIdsOn = mInputManager.GetArray(6);
            auto noteIdsOff = mInputManager.GetArray(7);

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

        auto output0 = outputs.at(0).GetIterator(numSamples, gArpeggioUpdateRate);
        auto output1 = outputs.at(1).GetIterator(numSamples, gArpeggioUpdateRate);
        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mInputManager.NodeUpdate(inputs, mUpdateRate);
                mUpdateRate = mInputManager.GetValueNext(1);

                UpdateSignal(mInputManager);
                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float envelope = mInputManager.GetValueNext(100);
                    float gain = mGainTarget * envelope;
                    mGain += 0.0025f * (gain - mGain);

                    mFreq += mFreqSmoothing * (mFreqTarget - mFreq);

                    *output0++ = mFreq;
                    *output1++ = mGain;
                }
            }
        );
    }

}
