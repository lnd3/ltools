#include "nodegraph/operations/NodeGraphOpSignal.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphSignalBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mNodeInputManager.ProcessUpdate(inputs, numSamples, mUpdateRate);

        float sync = mNodeInputManager.GetValueNext(0);
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        float* output0 = &outputs.at(0).Get(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mUpdateRate = inputs.at(1).Get();
                mNodeInputManager.NodeUpdate(inputs, mUpdateRate);

                mSmooth = mNodeInputManager.GetValueNext(4);

                mNodeInputManager.SetDuration(2, 10000.0f * (1.0f - mSmooth), 0.05f);

                mDeltaTime = 1.0f / 44100.0f;

                UpdateSignal(inputs, outputs);

                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float signalTarget = ProcessSignal(mDeltaTime, mNodeInputManager.GetValueNext(2));
                    *output0++ = mNodeInputManager.GetValueNext(3) * signalTarget;
                }
            }
        );
    }

    /*********************************************************************/

    void GraphSignalSine2::ResetInput() {
        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInput(mNumDefaultInputs + 1, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(mNumDefaultInputs + 1, InputBound::INPUT_0_TO_1);
        mUpdateRate = 16.0f;
    }

    void GraphSignalSine2::ResetSignal() {
        mPhaseFmod = 0.0f;
        mPhase = 0.0f;
    }

    void GraphSignalSine2::UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {
        mFmod = mNodeInputManager.GetValueNext(mNumDefaultInputs + 0);
        mPmod = mNodeInputManager.GetValueNext(mNumDefaultInputs + 1);
        mFmod *= 0.25f * 0.25f * 0.5f * 44100.0f / l::math::functions::max(mFreq, 1.0f);

        mFilterFmod.SetConvergenceFactor().SetTarget(mFmod);
        mFilterPmod.SetConvergenceFactor().SetTarget(mPmod);
    }

    float GraphSignalSine2::ProcessSignal(float deltaTime, float freq) {
        mPhaseFmod += deltaTime * freq;
        mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0f);
        float fmod = mFilterFmod.Next();
        float modulation = 0.5f * (fmod + fmod * l::math::functions::cos(l::math::constants::PI_f * mPhaseFmod * 2.0f));
        //float modulation = (0.5f - deltaPhase) + (mFmod) * 0.5f * l::math::functions::cos(l::math::constants::PI_f * mPhaseFmod * 2.0f);


        mPhase = mPhaseFmod;
        mPhase += modulation;
        mPhase -= l::math::functions::floor(mPhase);

        float phaseMod = mPhaseFmod + mFilterPmod.Next() * modulation * 4.0f;
        phaseMod -= l::math::functions::floor(phaseMod);

        return 0.5f * (l::math::functions::sin(l::math::constants::PI_f * mPhase * 2.0f) + l::math::functions::sin(l::math::constants::PI_f * phaseMod * 2.0f));
    }

    /*********************************************************************/

    void GraphSignalSaw2::ResetInput() {
        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInput(mNumDefaultInputs + 1, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(mNumDefaultInputs + 1, InputBound::INPUT_0_TO_1);
        mUpdateRate = 16.0f;
    }

    void GraphSignalSaw2::ResetSignal() {
        InitSaw(&mSaw, 0.0, 0.0);
    }

    void GraphSignalSaw2::UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {
        mAttenuation = mNodeInputManager.GetValueNext(mNumDefaultInputs + 0);
        mCutoff = mNodeInputManager.GetValueNext(mNumDefaultInputs + 1);

        UpdateSaw(&mSaw, 0.00001f + 0.99999f * mAttenuation * mAttenuation * mAttenuation, mCutoff * mCutoff);
    }

    float GraphSignalSaw2::ProcessSignal(float deltaTime, float freq) {
        return static_cast<float>(ProcessSaw(&mSaw, static_cast<double>(deltaTime) * static_cast<double>(freq)));
    }

    /*********************************************************************/
    void GraphSignalSine::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;

        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(5).Get();

                if (mFreq == 0.0f) {
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return updateRate;
                }
                if (mReset > 0.5f) {
                    mVolume = 0.0f;
                }
                mDeltaTime = 1.0 / 44100.0;

                return updateRate;
            },
            [&](int32_t start, int32_t end, bool) {

                mFmod = static_cast<double>(inputs.at(2).Get());
                mPmod = static_cast<double>(inputs.at(3).Get());
                double smooth = 0.5 * static_cast<double>(inputs.at(4).Get());

                double fmodRange = 16.0;
                mFmod = mFmod > 1.0 ? fmodRange * (mFmod - 1.0) : 1.0 / (1.0 + fmodRange * (1.0 - mFmod));

                //double limitFmMod = 1.0 / l::math::functions::max(mFreq / 25.0, 1.0);
                //mPmod = 800.0 * fmMod * fmMod * limitFmMod;

                //double limitFmMod = 1.0 / l::math::functions::max(mFreq / 25.0, 1.0);
                //mFmod = 800.0 * mFmod * mFmod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta = mDeltaTime * mFreq;

                    mPhaseFmod += phaseDelta;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    double modulation = mFmod * (1.0 + 0.5 * l::math::functions::sin(l::math::constants::PI * mPhaseFmod * 2.0));

                    mPhase += phaseDelta * modulation;
                    mPhase = l::math::functions::mod(mPhase, 1.0);

                    double phaseMod = mPhase + mPmod;
                    phaseMod = l::math::functions::mod(phaseMod, 1.0);

                    mWave += smooth * (mPhase + phaseMod - mWave);
                    double sine = l::math::functions::sin(l::math::constants::PI * mWave);

                    /*
                    double phaseDelta = mDeltaTime * mFreq;
                    mPhase += phaseDelta * mFmod;
                    mPhase = l::math::functions::mod(mPhase, 1.0);

                    double phaseMod = mPhase + mPmod;
                    phaseMod = l::math::functions::mod(phaseMod, 1.0);

                    double sine = l::math::functions::sin(l::math::constants::PI * (mPhase + phaseMod));
                    */

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(sine);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSignalSineFM::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(8).Get();

                if (mReset > 0.0f || mVolume + mVol < 0.0000001f) {
                    mPhase = 0.0;
                    mPhaseFmod = 0.0;
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return updateRate;
                }
                mDeltaTime = 1.0 / 44100.0;

                return updateRate;
            },
            [&](int32_t start, int32_t end, bool) {

                mFmod = static_cast<double>(inputs.at(2).Get());
                mFmodFrq = static_cast<double>(inputs.at(3).Get());
                mFmodVol = static_cast<double>(inputs.at(4).Get());
                mFmodOfs = static_cast<double>(inputs.at(5).Get());
                double fmodGain = static_cast<double>(inputs.at(6).Get());
                double smooth = 0.5 * static_cast<double>(inputs.at(7).Get());

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

                    mWave += smooth * (mPhase - mWave);
                    double out = l::math::functions::sin(l::math::constants::PI * mWave * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(out);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSignalSineFM2::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(5).Get();

                if (mReset > 0.0f || mVolume + mVol < 0.0000001f) {
                    mPhase = 0.0;
                    mPhaseFmod = 0.0;
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return updateRate;
                }
                mDeltaTime = 1.0 / 44100.0;
                mDeltaLimit = mDeltaTime * 2.0;

                return updateRate;
            },
            [&](int32_t start, int32_t end, bool) {

                double fmMod = static_cast<double>(inputs.at(2).Get());
                double fmFreq = static_cast<double>(inputs.at(3).Get());
                double smooth = 0.5 * static_cast<double>(inputs.at(4).Get());

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
                    mWave += smooth * (mPhase - mWave);

                    double out = l::math::functions::sin(l::math::constants::PI * mWave * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(out);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSignalSineFM3::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(4).Get();

                if (mReset > 0.0f || mVolume < 0.0000001f) {
                    mPhase = 0.0;
                    mPhaseFmod = 0.0;
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return updateRate;
                }
                mDeltaTime = 1.0 / 44100.0;
                mDeltaLimit = mDeltaTime * 4.0;

                return updateRate;
            },
            [&](int32_t start, int32_t end, bool) {

                double fmMod = static_cast<double>(inputs.at(2).Get());
                double smooth = 0.5 * static_cast<double>(inputs.at(3).Get());
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

                    mWave += smooth * (mPhase - mWave);
                    double out = l::math::functions::sin(l::math::constants::PI * mWave * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(out);
                }
            }
        );
    }

    /*********************************************************************/
    void GraphSignalSaw::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(5).Get();

                if (mFreq == 0.0f) {
                    mVolume = 0.0f;
                    outputs.at(0).mOutput = 0.0f;
                    return updateRate;
                }
                if (mReset > 0.5f) {
                    mVolume = 0.0f;
                }
                mDeltaTime = 1.0 / 44100.0;

                return updateRate;
            },
            [&](int32_t start, int32_t end, bool) {

                mFmod = static_cast<double>(inputs.at(2).Get());
                mPmod = static_cast<double>(inputs.at(3).Get());
                double smooth = 0.5 * static_cast<double>(inputs.at(4).Get());

                double fmodRange = 16.0;
                mFmod = mFmod > 1.0 ? fmodRange * (mFmod - 1.0) : 1.0 / (1.0 + fmodRange * (1.0 - mFmod));

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta = mDeltaTime * mFreq;

                    mPhaseFmod += phaseDelta;
                    mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0);
                    double modulation = mFmod * mPhaseFmod;

                    mPhase += phaseDelta * modulation;
                    mPhase = l::math::functions::mod(mPhase, 1.0);

                    double phaseMod = mPhase + mPmod;
                    phaseMod = l::math::functions::mod(phaseMod, 1.0);

                    double saw = mPhase + phaseMod;
                    mWave += smooth * (saw - mWave);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(mWave - 1.0);
                }
            }
        );
    }
}
