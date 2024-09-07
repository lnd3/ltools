#include "nodegraph/NodeGraphOperations.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    /*********************************************************************/
    void GraphSourceConstants::Reset() {
        switch (mMode) {
        case 0:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_0_TO_1);
            }
            break;
        case 1:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_NEG_1_POS_1);
            }
            break;
        case 2:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_0_100);
            }
            break;
        default:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_UNBOUNDED);
            }
            break;
        }
    }

    void GraphSourceConstants::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (int8_t i = 0; i < mNumOutputs; i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphSourceConstants::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    /*********************************************************************/
    void GraphSourceTime::Process(int32_t, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>& outputs) {
        float rate = 44100.0f;
        float phaseChange = 1.0f / rate;
        mAudioTime += phaseChange;

        outputs.at(0).mOutput = mAudioTime;
        outputs.at(1).mOutput = mFrameTime;
    }

    void GraphSourceTime::Tick(int32_t, float deltaTime) {
        mFrameTime += deltaTime;
    }

    void GraphSourceTime::Reset() {
        mAudioTime = 0.0f;
        mFrameTime = 0.0f;
    }

    /*********************************************************************/
    void GraphSourceSine::Reset() {
        // { "Freq Hz", "Freq Mod", "Phase Mod", "Reset"};
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.0f);
        mNode->SetInput(2, 1.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(3, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
    }

    void GraphSourceSine::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNote = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(4).Get();

                if (mNote == 0.0f) {
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return;
                }
                if (mReset > 0.5f) {
                    mVolume = 0.0f;
                }
                mDeltaTime = 1.0 / 44100.0;

            },
            [&](int32_t start, int32_t end, bool) {

                mFmod = static_cast<double>(inputs.at(2).Get());
                mPmod = static_cast<double>(inputs.at(3).Get());

                //double limitFmMod = 1.0 / l::math::functions::max(mNote / 25.0, 1.0);
                //mPmod = 800.0 * fmMod * fmMod * limitFmMod;

                double limitFmMod = 1.0 / l::math::functions::max(mNote / 25.0, 1.0);
                mFmod = 800.0 * mFmod * mFmod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta = mDeltaTime * mNote;

                    mPhase += phaseDelta * (1.0 + mFmod);
                    mPhase = l::math::functions::mod(mPhase, 1.0);

                    mPhaseMod += mPhase + mPmod;
                    mPhaseMod = l::math::functions::mod(mPhaseMod, 1.0);

                    double sine = l::math::functions::sin(l::math::constants::PI * (mPhase + mPhaseMod));

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(sine);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSourceSineFM::Reset() {
        // { "Note", "Volume", "Fmod", "FmodFreq", "FmodVol", "FmodOfs", "Reset"}
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 0.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.0f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInput(6, 0.0f);
        mNode->SetInput(7, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(3, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(4, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(5, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(6, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(7, InputBound::INPUT_0_TO_1);
    }

    void GraphSourceSineFM::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNote = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(7).Get();

                if (mReset > 0.0f || mVolume + mVol < 0.0000001f) {
                    mPhase = 0.0;
                    mPhaseFmod = 0.0;
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return;
                }
                mDeltaTime = 1.0 / 44100.0;

            },
            [&](int32_t start, int32_t end, bool) {

                mFmod = static_cast<double>(inputs.at(2).Get());
                mFmodFrq = static_cast<double>(inputs.at(3).Get());
                mFmodVol = static_cast<double>(inputs.at(4).Get());
                mFmodOfs = static_cast<double>(inputs.at(5).Get());
                double fmodGain = static_cast<double>(inputs.at(6).Get());

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta2 = mDeltaTime * mNote * mFmodFrq;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    double modWave = fmodGain * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 0.5);
                    double fmod = (mFmodOfs + 1.0) * mFmodVol + mFmodVol * modWave;
                    fmod = l::math::functions::clamp(fmod, 0.0, 500.0);

                    double phaseDelta = mDeltaTime * mNote * (fmod + 1.0) * (mFmod + 1.0);
                    phaseDelta = l::math::functions::clamp(phaseDelta, phaseDelta2, 0.5);
                    mPhase += phaseDelta;
                    mPhase = l::math::functions::mod(mPhase, 1.0);
                    double waveTarget = l::math::functions::sin(l::math::constants::PI * mPhase * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);
                    mWave += (waveTarget - mWave) * 0.5;

                    *output0++ = mVol * static_cast<float>(mWave);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSourceSineFM2::Reset() {
        // { "Note", "Volume", "FmodVol", "FmodOfs", "Reset"}
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 0.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_100);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
    }

    void GraphSourceSineFM2::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNote = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(4).Get();

                if (mReset > 0.0f || mVolume + mVol < 0.0000001f) {
                    mPhase = 0.0;
                    mPhaseFmod = 0.0;
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return;
                }
                mDeltaTime = 1.0 / 44100.0;
                mDeltaLimit = mDeltaTime * 2.0;

            },
            [&](int32_t start, int32_t end, bool) {

                double fmMod = static_cast<double>(inputs.at(2).Get());
                double fmFreq = static_cast<double>(inputs.at(3).Get());

                double limitFmMod = 1.0 / l::math::functions::max(mNote / 25.0, 1.0);
                fmMod = 800.0 * fmMod * fmMod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double maxFmModulation = 1.0 / l::math::functions::max(mNote * fmFreq * mDeltaLimit, 1.0);
                    maxFmModulation *= maxFmModulation;
                    maxFmModulation *= maxFmModulation;
                    fmFreq = fmFreq * maxFmModulation;

                    double fmNote = mNote * fmFreq;
                    double phaseDelta2 = mDeltaTime * fmNote;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    mFmod = (fmMod + fmMod * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 2.0));

                    double phaseDelta = mDeltaTime * mNote * (mFmod + 1.0) / (fmMod + 1.0);

                    mPhase += phaseDelta;
                    mPhase = l::math::functions::mod(mPhase, 1.0);
                    double waveTarget = l::math::functions::sin(l::math::constants::PI * mPhase * 2.0);


                    mVol += (1.0f / 256.0f) * (mVolume - mVol);
                    mWave += (waveTarget - mWave) * 0.5;

                    *output0++ = mVol * static_cast<float>(mWave);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSourceSineFM3::Reset() {
        // { "Note", "Volume", "Fmod", "Reset"}
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 0.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
    }

    void GraphSourceSineFM3::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mNote = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(3).Get();

                if (mReset > 0.0f || mVolume < 0.0000001f) {
                    mPhase = 0.0;
                    mPhaseFmod = 0.0;
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return;
                }
                mDeltaTime = 1.0 / 44100.0;
                mDeltaLimit = mDeltaTime * 4.0;
            },
            [&](int32_t start, int32_t end, bool) {

                double fmMod = static_cast<double>(inputs.at(2).Get());

                double limitFmMod = 1.0 / l::math::functions::max(mNote / 25.0, 1.0);
                double fm = 800.0 * fmMod * fmMod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta2 = mDeltaTime * mNote;
                    mPhaseFmod += 0.25 * phaseDelta2;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    double modulation = fm * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 2.0);

                    double phaseDelta = mDeltaTime * mNote * modulation;
                    mPhase += phaseDelta;
                    mPhase = l::math::functions::mod(mPhase, 1.0);
                    double waveTarget = l::math::functions::sin(l::math::constants::PI * mPhase * 2.0);


                    mVol += (1.0f / 256.0f) * (mVolume - mVol);
                    mWave += (waveTarget - mWave) * 0.5;

                    *output0++ = mVol * static_cast<float>(mWave);
                }
            }
        );
    }

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
        float noteTarget = inputs.at(0).Get();
        float velocity = l::math::functions::pow(inputs.at(1).Get(), 0.5f);
        float attackFrames = inputs.at(2).Get() * 44100.0f / 1000.0f;
        float releaseFrames = inputs.at(3).Get() * 44100.0f / 1000.0f;
        float noteFade = inputs.at(4).Get();
        //noteFade = l::math::functions::pow(0.01f, 1.0f / (1000.0f * inputs.at(3).Get() * 44100.0f * 0.001f));

        if (noteTarget != 0.0f && mFrameCount < attackFrames) {
            if (mFrameCount == 0) {
                mNote = noteTarget;
            }
            // attack
            mNote = noteTarget;
            mFrameCount++;
        }

        if (noteTarget != 0.0f) {
            if (mEnvelopeTarget < velocity) {
                mEnvelopeTarget += velocity / attackFrames;
            }
            else {
                mEnvelopeTarget = velocity;
            }
            mNote += noteFade * noteFade * (noteTarget - mNote);
            //mNote = noteFade * (mEnvelope - noteTarget) + noteTarget;
        }
        else {
            // release
            if (mFrameCount > 0) {
                mEnvelopeTarget -= velocity / releaseFrames;
                if (mEnvelopeTarget < 0.0f) {
                    mEnvelopeTarget = 0.0f;
                    mFrameCount = 0;
                }
            }
            else {
                mEnvelopeTarget = 0.0f;
                mFrameCount = 0;
            }
        }

        mEnvelope += 0.1f * (mEnvelopeTarget - mEnvelope);
        outputs.at(0).mOutput = mNote;
        outputs.at(1).mOutput = mEnvelope * mEnvelope;
    }

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
    void GraphInputKeyboardPiano::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphInputKeyboardPiano::Tick(int32_t, float) {
        mKeyboard.Update();
    }

    void GraphInputKeyboardPiano::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
    }

    void GraphInputKeyboardPiano::NoteOn(int32_t note) {
        float frequency = l::audio::GetFrequencyFromNote(static_cast<float>(note));
        int8_t channel = GetNextNoteChannel(note);
        mNode->SetInput(static_cast<int8_t>(channel), frequency);
    }
    void GraphInputKeyboardPiano::NoteOff() {
        Reset();
    }

    void GraphInputKeyboardPiano::NoteOff(int32_t note) {
        int8_t channel = ResetNoteChannel(note);
        if (channel >= 0) {
            mNode->SetInput(channel, 0.0f);
        }
    }

    int8_t GraphInputKeyboardPiano::ResetNoteChannel(int32_t note) {
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == note) {
                mChannel.at(i).second = 0;
                return static_cast<int8_t>(i);
            }
        }
        // It is possible to get a note off for a note not playing because the channel was taken for another newer note
        return -1;
    }

    int8_t GraphInputKeyboardPiano::GetNextNoteChannel(int32_t note) {
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == note) {
                mChannel.at(i).second = mNoteCounter++;
                return static_cast<int8_t>(i);
            }
        }

        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == 0) {
                mChannel.at(i).first = note;
                mChannel.at(i).second = mNoteCounter++;
                return static_cast<int8_t>(i);
            }
        }

        int32_t lowestCount = INT32_MAX;
        int8_t lowestCountIndex = 0;
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (lowestCount > mChannel.at(i).second) {
                lowestCount = mChannel.at(i).second;
                lowestCountIndex = static_cast<int8_t>(i);
            }
        }
        mChannel.at(lowestCountIndex).first = note;
        mChannel.at(lowestCountIndex).second = mNoteCounter++;
        return lowestCountIndex;
    }

    /*********************************************************************/
    void GraphInputMidiKeyboard::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphInputMidiKeyboard::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
    }

    void GraphInputMidiKeyboard::MidiEvent(const l::hid::midi::MidiData& data) {
        //LOG(LogInfo) << "listener 1: dev" << data.device << " stat " << data.status << " ch " << data.channel << " d1 " << data.data1 << " d2 " << data.data2;

        if (data.status == 9) {
            // note on
            NoteOn(data.data1, data.data2);
        }
        else if (data.status == 8) {
            // note off
            NoteOff(data.data1);
        }
    }

    void GraphInputMidiKeyboard::NoteOn(int32_t note, int32_t velocity) {
        float frequency = l::audio::GetFrequencyFromNote(static_cast<float>(note));
        int8_t channel = GetNextNoteChannel(note);
        mNode->SetInput(static_cast<int8_t>(channel), frequency);
        mNode->SetInput(static_cast<int8_t>(1), velocity / 128.0f);
    }
    void GraphInputMidiKeyboard::NoteOff() {
        Reset();
    }

    void GraphInputMidiKeyboard::NoteOff(int32_t note) {
        int8_t channel = ResetNoteChannel(note);
        if (channel >= 0) {
            mNode->SetInput(channel, 0.0f);
        }
    }

    int8_t GraphInputMidiKeyboard::ResetNoteChannel(int32_t note) {
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == note) {
                mChannel.at(i).second = 0;
                return static_cast<int8_t>(i);
            }
        }
        // It is possible to get a note off for a note not playing because the channel was taken for another newer note
        return -1;
    }

    int8_t GraphInputMidiKeyboard::GetNextNoteChannel(int32_t note) {
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == note) {
                mChannel.at(i).second = mNoteCounter++;
                return static_cast<int8_t>(i);
            }
        }

        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == 0) {
                mChannel.at(i).first = note;
                mChannel.at(i).second = mNoteCounter++;
                return static_cast<int8_t>(i);
            }
        }

        int32_t lowestCount = INT32_MAX;
        int8_t lowestCountIndex = 0;
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (lowestCount > mChannel.at(i).second) {
                lowestCount = mChannel.at(i).second;
                lowestCountIndex = static_cast<int8_t>(i);
            }
        }
        mChannel.at(lowestCountIndex).first = note;
        mChannel.at(lowestCountIndex).second = mNoteCounter++;
        return lowestCountIndex;
    }

    /*********************************************************************/
    void GraphInputMidiKnobs::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphInputMidiKnobs::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
    }

    void GraphInputMidiKnobs::MidiEvent(const l::hid::midi::MidiData& data) {
        if (data.status == 11) {
            //LOG(LogInfo) << "listener 1: dev" << data.device << " stat " << data.status << " ch " << data.channel << " d1 " << data.data1 << " d2 " << data.data2;

            if (data.data1 >= 48 && data.data1 <= 55) {
                mNode->SetInput(static_cast<int8_t>(data.data1 - 48), data.data2 / 128.0f);
            }
        }
    }

}
