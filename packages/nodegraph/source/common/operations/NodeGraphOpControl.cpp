#include "nodegraph/operations/NodeGraphOpControl.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"
#include "math/MathSmooth.h"

#include <math.h>

namespace l::nodegraph {
    /*********************************************************************/

    void GraphControlBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mNodeInputManager.BatchUpdate(inputs, numSamples);

        float sync = mNodeInputManager.GetValueNext(0);
        mUpdateRate = mNodeInputManager.GetValueNext(1);
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        auto output0 = outputs.at(0).GetIterator(numSamples, 16.0f);
        auto output1 = outputs.at(1).GetIterator(numSamples, 16.0f);


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

    void GraphControlEnvelope::UpdateSignal(NodeInputManager&) {

        mFreqTarget = mNodeInputManager.GetValueNext(6);
        if (mFreqTarget != 0.0f) {
            mNodeInputManager.SetTarget(100, mFreqTarget);
        }
        mNodeInputManager.SetDuration(100, 1000.0f * mNodeInputManager.GetValueNext(3));
        mAttackFrames = l::audio::GetAudioTicksFromMS(mNodeInputManager.GetValueNext(4));
        mReleaseFrames = l::audio::GetAudioTicksFromMS(mNodeInputManager.GetValueNext(5));
        mAttackFactor = mNodeInputManager.GetValueNext(4);
        mReleaseFactor = mNodeInputManager.GetValueNext(5);

        mNodeInputManager.SetUpdateRate(100, 1.0f);
        mNodeInputManager.SetUpdateRate(101, 1.0f);
    }

    std::pair<float, float> GraphControlEnvelope::ProcessSignal(NodeInputManager&) {
        float velocity = mNodeInputManager.GetValueNext(2);
        float freq = 0.0f;

        bool noteOn = mFreqTarget != 0.0f;
        bool differentNote = mNoteOn && noteOn && mFreqTargetPrev != mFreqTarget;
        if ((noteOn && !mNoteOn) || differentNote) {
            mNoteOn = true;

            mFreqTargetPrev = mFreqTarget;

            mNodeInputManager.SetDuration(101, mAttackFactor);
            mNodeInputManager.SetTarget(101, l::math::functions::sqrt(velocity));

            if (mFrameCount == 0) {
                // if note was off we set freq immediately
                freq = mFreqTarget;
            }
            mFrameCount = 0;
        }
        else if (!noteOn && mNoteOn) {
            mNoteOn = false;

            mNodeInputManager.SetDuration(101, mReleaseFactor);
            mNodeInputManager.SetTarget(101, 0.0f);

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

        freq = mNodeInputManager.GetValueNext(100);

        float envelope = mNodeInputManager.GetValueNext(101);
        return { freq, envelope };
    }

    /*********************************************************************/

    void GraphControlArpeggio::UpdateSignal(NodeInputManager&) {
        float attack = mNodeInputManager.GetValue(4);
        float release = mNodeInputManager.GetValue(5);
        float attackFactor = l::audio::GetRWAFactorFromMS(attack, 0.001f, gArpeggioUpdateRate);
        float releaseFactor = l::audio::GetRWAFactorFromMS(release, 0.001f, gArpeggioUpdateRate);
        mGainAttack = attackFactor;
        mGainRelease = releaseFactor;

        if (mNotes.empty()) {
            mGainTarget = 0.0f;
            mNodeInputManager.SetDuration(100, release, 0.01f);
            mNodeInputManager.SetDuration(101, release, 0.01f);
            mNodeInputManager.SetTarget(100, 0.0f);
            mNodeInputManager.SetTarget(101, 1.0f);
        }
        else {
            if (mUpdateCounter % 2 == 0) {
                mNoteIndex = mNoteIndex % mNotes.size();
                mFreqTarget = l::audio::GetFrequencyFromNote(static_cast<float>(mNotes.at(mNoteIndex)));
                mNoteIndex++;

                auto velocity = mNodeInputManager.GetValueNext(2);
                auto fade = mNodeInputManager.GetValueNext(3);

                mGainTarget = 2.0f * velocity;
                mFreqSmoothing = fade;
                mFreqSmoothing *= mFreqSmoothing * 0.5f;

                mNodeInputManager.SetDuration(100, attack, 0.01f);
                mNodeInputManager.SetDuration(101, attack + release, 0.01f);
                mNodeInputManager.SetTarget(100, 1.0f);
                mNodeInputManager.SetTarget(101, 0.0f);
                //inputManager.SetValue(100, 0.0f);
                //inputManager.SetValue(101, 1.0f);

                mUpdateCounter = 0;
            }
            else {
                mNodeInputManager.SetDuration(100, release, 0.01f);
                mNodeInputManager.SetDuration(101, release, 0.01f);
                mNodeInputManager.SetTarget(100, 0.0f);
                mNodeInputManager.SetTarget(101, 1.0f);
            }
            mUpdateCounter++;
        }
    }

    void GraphControlArpeggio::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mNodeInputManager.BatchUpdate(inputs, numSamples);
        float sync = mNodeInputManager.GetValueNext(0);
        float bpm = mNodeInputManager.GetValueNext(1);
        mUpdateRate = 44100.0f * 60.0f / (2.0f * 4.0f * bpm);
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

        auto output0 = outputs.at(0).GetIterator(numSamples, gArpeggioUpdateRate);
        auto output1 = outputs.at(1).GetIterator(numSamples, gArpeggioUpdateRate);
        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNodeInputManager.NodeUpdate(inputs, mUpdateRate);
                mUpdateRate = mNodeInputManager.GetValueNext(1);

                UpdateSignal(mNodeInputManager);
                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float envelope = mNodeInputManager.GetValueNext(100);
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
