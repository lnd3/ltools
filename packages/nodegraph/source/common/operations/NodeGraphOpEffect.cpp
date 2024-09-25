#include "nodegraph/operations/NodeGraphOpEffect.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"
#include "math/MathSmooth.h"

#include <math.h>

namespace l::nodegraph {
    /*********************************************************************/

    void GraphEffectBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float sync = inputs.at(0).Get();
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        auto input0 = inputs.at(4).GetIterator(numSamples);
        auto input1 = inputs.at(5).GetIterator(numSamples);
        auto output0 = outputs.at(0).GetIterator(numSamples);
        auto output1 = outputs.at(1).GetIterator(numSamples);

        mFilterGain.SetConvergenceFactor();
        mFilterMix.SetConvergenceFactor();

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mUpdateRate = inputs.at(1).Get();
                mFilterGain.SetTarget(inputs.at(2).Get());
                mFilterMix.SetTarget(inputs.at(3).Get());

                UpdateSignal(inputs, outputs);

                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float gain = mFilterGain.Next();
                    float mix = gain * mFilterMix.Next();
                    float antimix = gain * (1.0f - mix);

                    auto in0 = *input0++;
                    auto in1 = *input1++;
                    auto [out0, out1] = ProcessSignal(in0, in1);
                    *output0++ = mix * out0 + antimix * in0;
                    *output1++ = mix * out1 + antimix * in1;
                }
            }
        );
    }

    /*********************************************************************/
    void GraphEffectReverb2::UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        fb = 0.2f * (1.0f - inputs.at(mNumDefaultInputs+0).Get());

        float roomSize = inputs.at(mNumDefaultInputs+1).Get();
        mBufSizeLimit = GetFramesPerRoomSize(roomSize);

        d0 = inputs.at(mNumDefaultInputs+2).Get();
        fb0 = 0.5f * 0.5f * math::functions::max(inputs.at(mNumDefaultInputs+3).Get(), 1.0f);
        d1 = inputs.at(mNumDefaultInputs+4).Get();
        fb1 = 0.5f * 0.5f * math::functions::max(inputs.at(mNumDefaultInputs+5).Get(), 1.0f);
        d2 = inputs.at(mNumDefaultInputs+6).Get();
        fb2 = 0.5f * 0.5f * math::functions::max(inputs.at(mNumDefaultInputs+7).Get(), 1.0f);

        mDelay0 = (int(mBufIndex + d0 * mBufSizeLimit)) % mBufSizeLimit;
        mDelay1 = (int(mBufIndex + d1 * mBufSizeLimit)) % mBufSizeLimit;
        mDelay2 = (int(mBufIndex + d2 * mBufSizeLimit)) % mBufSizeLimit;
    }

    std::pair<float, float> GraphEffectReverb2::ProcessSignal(float value0, float value1) {
        float out0 = (fb1 * mBuf1[mDelay1] + fb0 * mBuf0[mDelay0] + fb2 * mBuf0[mDelay2]);
        float out1 = (fb1 * mBuf0[mDelay1] + fb0 * mBuf1[mDelay0] + fb2 * mBuf1[mDelay2]);

        mBuf0[mBufIndex] = fb * mBuf1[mBufIndex] - fb1 * mBuf1[mDelay1] - fb0 * mBuf0[mDelay0] - fb2 * mBuf0[mDelay2] + value0;
        mBuf1[mBufIndex] = fb * mBuf0[mBufIndex] - fb1 * mBuf0[mDelay1] - fb0 * mBuf1[mDelay0] - fb2 * mBuf1[mDelay2] + value1;

        mBufIndex = (mBufIndex + 1) % mBufSizeLimit;

        mDelay0 = (mDelay0 + 1) % mBufSizeLimit;
        mDelay1 = (mDelay1 + 1) % mBufSizeLimit;
        mDelay2 = (mDelay2 + 1) % mBufSizeLimit;

        return { out0, out1 };
    }

    /*********************************************************************/

    void GraphEffectReverb1::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto wet = &inputs.at(2).Get(numSamples);

        fb = 0.2f * (1.0f - inputs.at(3).Get());

        float roomSize = inputs.at(4).Get();
        uint32_t bufSizeLimit = GetFramesPerRoomSize(roomSize);

        d0 = inputs.at(5).Get();
        fb0 = 0.5f * 0.5f * math::functions::max(inputs.at(6).Get(), 1.0f);
        d1 = inputs.at(7).Get();
        fb1 = 0.5f * 0.5f * math::functions::max(inputs.at(8).Get(), 1.0f);
        d2 = inputs.at(9).Get();
        fb2 = 0.5f * 0.5f * math::functions::max(inputs.at(10).Get(), 1.0f);

        float dry = 1.0f - *wet;

        uint32_t delay0 = (int(bufIndex + d0 * bufSizeLimit)) % bufSizeLimit;
        uint32_t delay1 = (int(bufIndex + d1 * bufSizeLimit)) % bufSizeLimit;
        uint32_t delay2 = (int(bufIndex + d2 * bufSizeLimit)) % bufSizeLimit;
        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        outputs[0].mOutput = in0 * dry + (fb1 * buf1[delay1] + fb0 * buf0[delay0] + fb2 * buf0[delay2]) * *wet;
        outputs[1].mOutput = in1 * dry + (fb1 * buf0[delay1] + fb0 * buf1[delay0] + fb2 * buf1[delay2]) * *wet;

        buf0[bufIndex] = fb * buf1[bufIndex] - fb1 * buf1[delay1] - fb0 * buf0[delay0] - fb2 * buf0[delay2] + in0;
        buf1[bufIndex] = fb * buf0[bufIndex] - fb1 * buf0[delay1] - fb0 * buf1[delay0] - fb2 * buf1[delay2] + in1;

        bufIndex = (bufIndex + 1) % bufSizeLimit;

        delay0 = (delay0 + 1) % bufSizeLimit;
        delay1 = (delay1 + 1) % bufSizeLimit;
        delay2 = (delay2 + 1) % bufSizeLimit;
    }

    /*********************************************************************/

    void GraphEffectReverb3::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void GraphEffectLimiter::UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        mFilterPreamp.SetTarget(inputs.at(mNumDefaultInputs + 0).Get());
        mFilterLimit.SetTarget(inputs.at(mNumDefaultInputs + 1).Get());
        float attackMs = inputs.at(mNumDefaultInputs + 2).Get();
        float releaseMs = inputs.at(mNumDefaultInputs + 3).Get();
        mAttack = l::audio::GetRWAFactorFromMS(attackMs, 0.001f);
        mRelease = l::audio::GetRWAFactorFromMS(releaseMs, 0.001f);
    }

    std::pair<float, float> GraphEffectLimiter::ProcessSignal(float value0, float value1) {
        float limit = mFilterLimit.Next();
        float preamp = mFilterPreamp.Next();

        float inVal0 = preamp * value0;
        float inVal1 = preamp * value1;
        float inVal = inVal0 > inVal1 ? inVal0 : inVal1;
        if (inVal > mEnvelope) {
            mEnvelope = mAttack * (mEnvelope - inVal) + inVal;
        }
        else {
            mEnvelope = mRelease * (mEnvelope - inVal) + inVal;
        }

        float envelopeAbs = l::math::functions::abs(mEnvelope);
        float out0;
        float out1;
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
            out0 = value0;
            out1 = value1;
        }
        return { out0, out1 };
    }

    /*********************************************************************/
    void GraphEffectEnvelopeFollower::UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        mFilterAttack.SetConvergenceInMs(inputs.at(2).Get());
        mFilterAttack.SetConvergenceInMs(inputs.at(3).Get());
    }

    std::pair<float, float> GraphEffectEnvelopeFollower::ProcessSignal(float value0, float value1) {
        float inVal = value0 > value1 ? value0 : value1;
        float attack = mFilterAttack.Next();
        float release = mFilterRelease.Next();
        if (inVal > mEnvelope) {
            mEnvelope = attack * (mEnvelope - inVal) + inVal;
        }
        else {
            mEnvelope = release * (mEnvelope - inVal) + inVal;
        }

        float envelopeAbs = l::math::functions::abs(mEnvelope);
        return { envelopeAbs * value0, envelopeAbs * value1 };
    }

    /*********************************************************************/

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

        float updateRate = 44100.0f * 60.0f / bpm;

        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate * fmodPerPattern, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mGateIndex %= gate.size();
                mGainTarget = gate[mGateIndex];
                mGateIndex++;

                return updateRate * fmodPerPattern;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float in0 = inputs.at(0).Get();
                    float in1 = inputs.at(1).Get();

                    float delta = mGainTarget - mGain;
                    if (delta > 0) {
                        mGain += mGateSmoothing * l::math::smooth::smoothPolyh3(delta);
                    }
                    else {
                        mGain += mGateSmoothingNeg * (-l::math::smooth::smoothPolyh3(-delta));
                    }

                    outputs.at(0).mOutput = mGain * in0;
                    outputs.at(1).mOutput = mGain * in1;
                }
            });
    }
}
