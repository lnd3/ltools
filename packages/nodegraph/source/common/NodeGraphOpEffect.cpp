#include "nodegraph/NodeGraphOpEffect.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"
#include "math/MathSmooth.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphEffectReverb1::Reset() {
        // { "In 1", "In 2", "Mix", "Attenuation", "Room Size", "Delay 1", "Feedback 1", "Delay 2", "Feedback 2", "Delay 3", "Feedback 3" };

        mNode->SetInput(2, 0.75f);
        mNode->SetInput(3, 0.5f);
        mNode->SetInput(4, 30.0f);
        mNode->SetInput(5, 0.5f);
        mNode->SetInput(6, 0.9f);
        mNode->SetInput(7, 0.8f);
        mNode->SetInput(8, 0.9f);
        mNode->SetInput(9, 0.7f);
        mNode->SetInput(10, 0.9f);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(4, InputBound::INPUT_CUSTOM, 0.2f, maxRoomSizeInMeters);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(6, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(7, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(8, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(9, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(10, InputBound::INPUT_0_TO_1);
    }

    void GraphEffectReverb1::Process(int32_t, std::vector<NodeGraphInput>&inputs, std::vector<NodeGraphOutput>&outputs) {
        float wet = inputs.at(2).Get();
        
        fb = 0.2f * (1.0f - inputs.at(3).Get());

        float roomSize = inputs.at(4).Get();
        uint32_t bufSizeLimit = GetFramesPerRoomSize(roomSize);

        d0 = inputs.at(5).Get();
        fb0 = 0.5f * 0.5f * math::functions::max(inputs.at(6).Get(), 1.0f);
        d1 = inputs.at(7).Get();
        fb1 = 0.5f * 0.5f * math::functions::max(inputs.at(8).Get(), 1.0f);
        d2 = inputs.at(9).Get();
        fb2 = 0.5f * 0.5f * math::functions::max(inputs.at(10).Get(), 1.0f);

        float dry = 1.0f - wet;

        uint32_t delay0 = (int(bufIndex + d0 * bufSizeLimit)) % bufSizeLimit;
        uint32_t delay1 = (int(bufIndex + d1 * bufSizeLimit)) % bufSizeLimit;
        uint32_t delay2 = (int(bufIndex + d2 * bufSizeLimit)) % bufSizeLimit;
        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        outputs[0].mOutput = in0 * dry + (fb1 * buf1[delay1] + fb0 * buf0[delay0] + fb2 * buf0[delay2]) * wet;
        outputs[1].mOutput = in1 * dry + (fb1 * buf0[delay1] + fb0 * buf1[delay0] + fb2 * buf1[delay2]) * wet;

        buf0[bufIndex] = fb * buf1[bufIndex] - fb1 * buf1[delay1] - fb0 * buf0[delay0] - fb2 * buf0[delay2] + in0;
        buf1[bufIndex] = fb * buf0[bufIndex] - fb1 * buf0[delay1] - fb0 * buf1[delay0] - fb2 * buf1[delay2] + in1;

        bufIndex = (bufIndex + 1) % bufSizeLimit;

        delay0 = (delay0 + 1) % bufSizeLimit;
        delay1 = (delay1 + 1) % bufSizeLimit;
        delay2 = (delay2 + 1) % bufSizeLimit;
    }

    /*********************************************************************/

    void GraphEffectReverb2::Reset() {
        // { "In 1", "In 2", "Mix", "Feedback", "Room Size", "Width", "First tap", "Longest tap", "Num taps", "Tap bulge", "Filter cutoff", "Filter res"};
        mNode->SetInput(2, 0.3f);
        mNode->SetInput(3, 0.5f);
        mNode->SetInput(4, 30.0f);
        mNode->SetInput(5, 0.5f);
        mNode->SetInput(6, 0.1f);
        mNode->SetInput(7, 0.8f);
        mNode->SetInput(8, 5.0f);
        mNode->SetInput(9, 0.7f);
        mNode->SetInput(10, 0.95f);
        mNode->SetInput(11, 0.01f);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(4, InputBound::INPUT_CUSTOM, 1.0f, 334.0f);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(6, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
        mNode->SetInputBound(7, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
        mNode->SetInputBound(8, InputBound::INPUT_CUSTOM, 1.0f, 30.0f);
        mNode->SetInputBound(9, InputBound::INPUT_CUSTOM, 1.0f, 10.0f);
        mNode->SetInputBound(10, InputBound::INPUT_CUSTOM, 0.001f, 0.999f);
        mNode->SetInputBound(11, InputBound::INPUT_CUSTOM, 0.001f, 0.999f);
    }

    void GraphEffectReverb2::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float wet = inputs.at(2).Get();
        float reverbFeedback = inputs.at(3).Get();
        float roomSize = inputs.at(4).Get();
        float stereoWidth = inputs.at(5).Get();
        float earliestDelay = inputs.at(6).Get();
        float longestDelay = inputs.at(7).Get();
        float numDelays = inputs.at(8).Get();
        float tapBulge = inputs.at(9).Get();
        float cutoff = inputs.at(10).Get();
        float res = inputs.at(11).Get();

        if (roomSize > maxRoomSizeInMeters) {
            roomSize = maxRoomSizeInMeters;
            mNode->SetInput(3, maxRoomSizeInMeters);
        }
        else if (roomSize < 0.1f) {
            roomSize = 0.1f;
            mNode->SetInput(3, 0.1f);
        }
        uint32_t bufSizeLimit = GetFramesPerRoomSize(roomSize);

        // feedback and delay

        float dry = 1.0f - wet;

        // indexes for delays with at least one index back
        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        auto echoStrength = [](float x, float earlyBulgeStrength) {
            // f(x) = E * 2 * x * pow(E, -x * 2);
            return 2.72f * x * earlyBulgeStrength * powf(2.72f, -x * earlyBulgeStrength);
            };

        auto sample = [&](float* buf0, float* buf1, float earliestDelay, float longestDelay, float numDelays, float attenuation, float stereoWidth) {
            float delayLength = longestDelay - earliestDelay;
            float delayValue0 = 0.0f;
            float delayValue1 = 0.0f;

            float attenuationSum = 0.01f;
            float stereoMixing = stereoWidth;
            for (int i = 0; i < static_cast<int32_t>(numDelays); i++) {
                //float random = mLCG() / static_cast<float>(INT32_MAX);

                float x = 0.5f / numDelays + i / numDelays;
                float delay0 = earliestDelay + delayLength * x * x;
                float delay1 = delay0;

                uint32_t delayIndex0 = (int(bufIndex + 2 * bufSizeLimit - 1 - delay0 * bufSizeLimit)) % bufSizeLimit;
                uint32_t delayIndex1 = (int(bufIndex + 2 * bufSizeLimit - 1 - delay1 * bufSizeLimit)) % bufSizeLimit;

                float monoMixing = 1.0f - stereoMixing;
                float gain = echoStrength(x, tapBulge);
                delayValue0 += gain * (stereoMixing * buf0[delayIndex0] + monoMixing * buf1[delayIndex0]);
                delayValue1 += gain * (stereoMixing * buf1[delayIndex1] + monoMixing * buf0[delayIndex1]);

                attenuationSum += gain;
            }

            float gainAdjust = attenuation / attenuationSum;

            // normalize reverb based on input attenuation value
            delayValue0 *= gainAdjust;
            delayValue1 *= gainAdjust;

            return std::tuple(delayValue0, delayValue1);
            };


        auto [delay0, delay1] = sample(bufEarlyTap0.data(), bufEarlyTap1.data(), earliestDelay, longestDelay, numDelays, reverbFeedback, stereoWidth);

        bufEarlyTap0[bufTapIndex] = reverbFeedback * (bufEarlyTap0[bufTapIndex] + delay0 + in0);
        bufEarlyTap1[bufTapIndex] = reverbFeedback * (bufEarlyTap1[bufTapIndex] + delay1 + in1);

        bufTapIndex = (bufTapIndex + 1) % bufSizeLimit;

        outputs[2].mOutput = in0 * dry + bufEarlyTap0[bufTapIndex] * wet;
        outputs[3].mOutput = in1 * dry + bufEarlyTap1[bufTapIndex] * wet;

        bufRev0[bufIndex] += delay0;
        bufRev1[bufIndex] += delay1;

        // buffer blur of oldest sample plus next oldest sample with global attenuation and new input
        auto [reverb0, reverb1] = sample(bufRev0.data(), bufRev1.data(), earliestDelay, longestDelay, numDelays, reverbFeedback, stereoWidth);

        cutoff *= cutoff;
        float rc = 1.0f - res * cutoff;

        mLP0 = rc * mLP0 - cutoff * (mLP1 + reverb0);
        mLP1 = rc * mLP1 + cutoff * mLP0;
        mLP2 = rc * mLP2 - cutoff * (mLP3 + reverb1);
        mLP3 = rc * mLP3 + cutoff * mLP2;

        bufRev0[bufIndex] = reverbFeedback * (bufRev0[bufIndex] + mLP1);
        bufRev1[bufIndex] = reverbFeedback * (bufRev1[bufIndex] + mLP3);

        bufIndex = (bufIndex + 1) % bufSizeLimit;

        outputs[0].mOutput = in0 * dry + bufRev0[bufIndex] * wet;
        outputs[1].mOutput = in1 * dry + bufRev1[bufIndex] * wet;

        // cross delay mixing (delay 1 & 2)
        //buf0[bufIndex] += fb1 * buf1[delay1];
        //buf1[bufIndex] += fb0 * buf0[delay0];

        // central delay mixing (delay 3)
        //float centralDelay = fb2 * (buf0[delay2] + buf1[delay2]);
        //buf0[bufIndex] += centralDelay;
        //buf1[bufIndex] += centralDelay;

        // 
    }

    /*********************************************************************/
    void GraphEffectLimiter::Reset() {
        // { "In 1", "In 2", "Attack", "Release", "Preamp", "Limit"};
        mEnvelope = 0.0f;
        mNode->SetInput(2, 5.0f);
        mNode->SetInput(3, 100.0f);
        mNode->SetInput(4, 1.0f);
        mNode->SetInput(5, 0.95f);
        mNode->SetInputBound(2, InputBound::INPUT_CUSTOM, 1.0f, 10000.0f);
        mNode->SetInputBound(3, InputBound::INPUT_CUSTOM, 1.0f, 10000.0f);
        mNode->SetInputBound(4, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
        mNode->SetInputBound(5, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
    }

    void GraphEffectLimiter::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float attackMs = inputs .at(2).Get();
        float releaseMs = inputs.at(3).Get();
        float attack = l::math::functions::pow(0.01f, 1.0f / (attackMs * 44100.0f * 0.001f));
        float release = l::math::functions::pow(0.01f, 1.0f / (releaseMs * 44100.0f * 0.001f));
        float preamp = inputs.at(4).Get();
        float limit = inputs.at(5).Get();

        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        float inVal0 = preamp * in0;
        float inVal1 = preamp * in1;
        float inVal = inVal0 > inVal1 ? inVal0 : inVal1;
        if (inVal > mEnvelope) {
            mEnvelope = attack * (mEnvelope - inVal) + inVal;
        }
        else {
            mEnvelope = release * (mEnvelope - inVal) + inVal;
        }

        float envelopeAbs = l::math::functions::abs(mEnvelope);
        if (envelopeAbs > limit) {
            if (envelopeAbs > 1.0f) {
                outputs.at(0).mOutput = inVal0 / mEnvelope;
                outputs.at(1).mOutput = inVal1 / mEnvelope;
            }
            else {
                outputs.at(0).mOutput = inVal0 / (1.0f + mEnvelope - limit);
                outputs.at(1).mOutput = inVal1 / (1.0f + mEnvelope - limit);
            }
        }
        else {
            outputs.at(0).mOutput = in0;
            outputs.at(1).mOutput = in1;
        }
        outputs.at(2).mOutput = envelopeAbs;
    }

    /*********************************************************************/
    void GraphEffectEnvelope::Reset() {
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

    void GraphEffectEnvelope::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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
            else if (freqTarget != 0) {
                mFrameCount = attackFrames + 2;
                mEnvelopeTarget = velocity;
            }
        }

        if (freqTarget != 0 && mFrameCount < attackFrames) {
            // attack
            mEnvelopeTarget = velocity;
            mFrameCount++;
        }
        else if (freqTarget != 0 && mFrameCount == attackFrames + 1) {
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

    /*********************************************************************/
    void GraphEffectEnvelopeFollower::Reset() {
        mEnvelope = 0.0f;
        mNode->SetInput(2, 5.0f);
        mNode->SetInput(3, 100.0f);
        mNode->SetInputBound(2, InputBound::INPUT_CUSTOM, 1.0f, 10000.0f);
        mNode->SetInputBound(3, InputBound::INPUT_CUSTOM, 1.0f, 10000.0f);
    }

    void GraphEffectEnvelopeFollower::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float attackMs = inputs.at(2).Get();
        float releaseMs = inputs.at(3).Get();
        float attack = l::math::functions::pow(0.01f, 1.0f / (attackMs * 44100.0f * 0.001f));
        float release = l::math::functions::pow(0.01f, 1.0f / (releaseMs * 44100.0f * 0.001f));

        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        float inVal = in0 > in1 ? in0 : in1;
        if (inVal > mEnvelope) {
            mEnvelope = attack * (mEnvelope - inVal) + inVal;
        }
        else {
            mEnvelope = release * (mEnvelope - inVal) + inVal;
        }

        float envelopeAbs = l::math::functions::abs(mEnvelope);
        outputs.at(0).mOutput = envelopeAbs;
    }

    /*********************************************************************/
    void GraphEffectSaturator::Reset() {
        // { "In 1", "In 2", "Wet", "Preamp", "Limit", "Postamp"};
        mEnvelope = 0.0f;
        mNode->SetInput(2, 0.5f);
        mNode->SetInput(3, 1.5f);
        mNode->SetInput(4, 0.6f);
        mNode->SetInput(5, 1.4f);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
        mNode->SetInputBound(4, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
        mNode->SetInputBound(5, InputBound::INPUT_CUSTOM, 0.0f, 10.0f);
    }

    void GraphEffectSaturator::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float wet = inputs.at(2).Get();
        float preamp = inputs.at(3).Get();
        float limit = inputs.at(4).Get();
        float postamp = inputs.at(5).Get();
        wet = postamp * wet;
        float dry = postamp * (1.0f - wet);

        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        float inPreamp0 = in0 * preamp;
        float inPreamp1 = in1 * preamp;

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

        outputs.at(0).mOutput = dry * in0 + wet * inPreamp0;
        outputs.at(1).mOutput = dry * in1 + wet * inPreamp1;
    }

    /*********************************************************************/
    void GraphEffectTranceGate::Reset() {
        // { "In 1", "In 2", "Bpm", "Fmod", "Attack", "Pattern"};

        mGateIndex = 0;

        mNode->SetInput(2, 60.0f);
        mNode->SetInput(3, 1.0f);
        mNode->SetInput(4, 0.001f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInput(6, 0.0f);
        mNode->SetInputBound(2, InputBound::INPUT_CUSTOM, 1.0f, 1000.0f);
        mNode->SetInputBound(3, InputBound::INPUT_CUSTOM, 0.01f, 1.0f);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(5, InputBound::INPUT_0_100);
        mNode->SetInputBound(6, InputBound::INPUT_0_TO_1);
    }

    void GraphEffectTranceGate::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        // "Bpm", "Fmod", "Attack", "Pattern"

        float bpm = inputs.at(2).Get();
        float fmod = inputs.at(3).Get();
        float attack = inputs.at(4).Get();

        if (inputs.at(6).Get() > 0.5f) {
            mSamplesUntilUpdate = 0.0f;
        }

        size_t patternsSize = patterns.size();
        int32_t patternId = static_cast<int32_t>(patternsSize * inputs.at(5).Get());
        auto& gate = patterns[patternId % patternsSize];

        size_t patternSize = gate.size();
        float fmodPerPattern = fmod / static_cast<float>(patternSize);

        mGateSmoothing = attack * attack;
        mGateSmoothingNeg = mGateSmoothing;

        float freq = 44100.0f * 60.0f / bpm;

        mSamplesUntilUpdate = l::audio::BatchUpdate(freq * fmodPerPattern, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mGateIndex %= gate.size();
                mGainTarget = gate[mGateIndex];
                mGateIndex++;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float in0 = inputs.at(0).Get();
                    float in1 = inputs.at(1).Get();

                    float delta = mGainTarget - mGain;
                    if (delta > 0) {
                        mGain += mGateSmoothing * l::math::smooth::smootPolyh3(delta);
                    }
                    else {
                        mGain += mGateSmoothingNeg * (-l::math::smooth::smootPolyh3(-delta));
                    }

                    outputs.at(0).mOutput = mGain * in0;
                    outputs.at(1).mOutput = mGain * in1;
                }
            });
    }

    /*********************************************************************/
    void GraphEffectArpeggio::Reset() {
        // { "Note On Id", "Note Off Id", "Velocity", "Bpm", "Fmod", "Attack"};

        mGainTarget = 0.0f;

        mNode->SetInput(0, l::audio::gNoNote_f, 8);
        mNode->SetInput(1, l::audio::gNoNote_f, 8);

        mNode->SetInput(2, 1.0f);
        mNode->SetInput(3, 60.0f);
        mNode->SetInput(4, 1.0f);
        mNode->SetInput(5, 0.01f);
        mNode->SetInput(6, 0.0f);

        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_CUSTOM, 1.0f, 1000.0f);
        mNode->SetInputBound(4, InputBound::INPUT_CUSTOM, 0.01f, 1.0f);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(6, InputBound::INPUT_0_TO_1);
    }

    void GraphEffectArpeggio::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        // "Note On Id", "Note Off Id", "Velocity", "Bpm", "Fmod", "Attack"

        auto noteIdsOn = &inputs.at(0).Get(8);
        auto noteIdsOff = &inputs.at(1).Get(8);
        float velocity = inputs.at(2).Get();
        float bpm = inputs.at(3).Get();
        float fmod = inputs.at(4).Get();
        float attack = inputs.at(5).Get();

        if (inputs.at(6).Get() > 0.5f) {
            mSamplesUntilUpdate = 0.0f;
        }

        if (!mNotes.empty()) {
            for (int32_t i = 0; i < 8; i++) {
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
            mSamplesUntilUpdate = 0.0f;
            mGainTarget = 0.0f;
        }
        for (int32_t i = 0; i < 8; i++) {
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

        mGainSmoothing = attack * attack;
        mGainSmoothingNeg = mGainSmoothing;

        float freq = 44100.0f * 60.0f / bpm;

        mSamplesUntilUpdate = l::audio::BatchUpdate(freq * fmod, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                if (mNotes.empty()) {
                    mGainTarget = 0.0f;
                }
                else {
                    mNoteIndex = mNoteIndex % mNotes.size();
                    mCurrentNoteFreq = l::audio::GetFrequencyFromNote(static_cast<float>(mNotes.at(mNoteIndex)));
                    mNoteIndex++;
                    mGainTarget = velocity;
                }
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float delta = mGainTarget - mGain;
                    if (delta > 0) {
                        mGain += mGainSmoothing * l::math::smooth::smootPolyh3(delta);
                    }
                    else {
                        mGain += mGainSmoothingNeg * (-l::math::smooth::smootPolyh3(-delta));
                    }

                    outputs.at(0).mOutput = mCurrentNoteFreq;
                    outputs.at(1).mOutput = mGain;
                }
            });
    }

}
