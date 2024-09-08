#include "nodegraph/NodeGraphOpSource.h"

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
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(4).Get();

                if (mFreq == 0.0f) {
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

                //double limitFmMod = 1.0 / l::math::functions::max(mFreq / 25.0, 1.0);
                //mPmod = 800.0 * fmMod * fmMod * limitFmMod;

                double limitFmMod = 1.0 / l::math::functions::max(mFreq / 25.0, 1.0);
                mFmod = 800.0 * mFmod * mFmod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta = mDeltaTime * mFreq;

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
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
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
                    double phaseDelta2 = mDeltaTime * mFreq * mFmodFrq;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    double modWave = fmodGain * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 0.5);
                    double fmod = (mFmodOfs + 1.0) * mFmodVol + mFmodVol * modWave;
                    fmod = l::math::functions::clamp(fmod, 0.0, 500.0);

                    double phaseDelta = mDeltaTime * mFreq * (fmod + 1.0) * (mFmod + 1.0);
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
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
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

                double limitFmMod = 1.0 / l::math::functions::max(mFreq / 25.0, 1.0);
                fmMod = 800.0 * fmMod * fmMod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double maxFmModulation = 1.0 / l::math::functions::max(mFreq * fmFreq * mDeltaLimit, 1.0);
                    maxFmModulation *= maxFmModulation;
                    maxFmModulation *= maxFmModulation;
                    fmFreq = fmFreq * maxFmModulation;

                    double fmNote = mFreq * fmFreq;
                    double phaseDelta2 = mDeltaTime * fmNote;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    mFmod = (fmMod + fmMod * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 2.0));

                    double phaseDelta = mDeltaTime * mFreq * (mFmod + 1.0) / (fmMod + 1.0);

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

        mSamplesUntilUpdate = l::audio::BatchUpdate(16.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
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

                double limitFmMod = 1.0 / l::math::functions::max(mFreq / 25.0, 1.0);
                double fm = 800.0 * fmMod * fmMod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta2 = mDeltaTime * mFreq;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    double modulation = fm * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 2.0);

                    double phaseDelta = mDeltaTime * mFreq * modulation;
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

}
