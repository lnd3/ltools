#include "nodegraph/operations/NodeGraphOpSignalGenerator.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void SignalGeneratorBase::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mInputManager.BatchUpdate(inputs, numSamples);

        float sync = mInputManager.GetValueNext(0);
        if (sync > 0.5f) {
            mSamplesUntilUpdate = 0;
        }

        float* output0 = &outputs.at(0).Get(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mUpdateRate = mInputManager.GetValueNext(1);
                mInputManager.NodeUpdate(inputs, mUpdateRate);

                mSmooth = mInputManager.GetValueNext(4);

                mRWAFreq.SetConvergenceInMs(1000.0f * (1.0f - mSmooth), 0.05f);

                mDeltaTime = 1.0f / 44100.0f;

                UpdateSignal(inputs, outputs);

                return mUpdateRate;
            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float freq = mInputManager.GetValueNext(2);
                    mRWAFreq.SetTarget(freq);
                    float signalTarget = ProcessSignal(mDeltaTime, freq);
                    *output0++ = mInputManager.GetValueNext(3) * signalTarget;
                }
            }
        );
    }

    /*********************************************************************/

    void SignalGeneratorSine2::DefaultDataInit() {
        NodeGraphOp::DefaultDataInit();

        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInput(mNumDefaultInputs + 1, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(mNumDefaultInputs + 1, InputBound::INPUT_0_TO_1);
        mUpdateRate = 16.0f;
    }

    void SignalGeneratorSine2::Reset() {
        mPhaseFmod = 0.0f;
        mPhase = 0.0f;
    }

    void SignalGeneratorSine2::UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {
        mFmod = mInputManager.GetValueNext(mNumDefaultInputs + 0);
        mPmod = mInputManager.GetValueNext(mNumDefaultInputs + 1);
        mFmod *= 0.25f * 0.25f * 0.5f * 44100.0f / l::math::max2(mFreq, 1.0f);

        mFilterFmod.SetConvergenceFactor().SetTarget(mFmod);
        mFilterPmod.SetConvergenceFactor().SetTarget(mPmod);
    }

    float SignalGeneratorSine2::ProcessSignal(float deltaTime, float freq) {
        mPhaseFmod += deltaTime * freq;
        mPhaseFmod = l::math::mod(mPhaseFmod, 1.0f);
        float fmod = mFilterFmod.Next();
        float modulation = 0.5f * (fmod + fmod * l::math::cos(l::math::constants::PI_f * mPhaseFmod * 2.0f));
        //float modulation = (0.5f - deltaPhase) + (mFmod) * 0.5f * l::math::cos(l::math::constants::PI_f * mPhaseFmod * 2.0f);


        mPhase = mPhaseFmod;
        mPhase += modulation;
        mPhase -= l::math::floor(mPhase);

        float phaseMod = mPhaseFmod + mFilterPmod.Next() * modulation * 4.0f;
        phaseMod -= l::math::floor(phaseMod);

        return 0.5f * (l::math::sin(l::math::constants::PI_f * mPhase * 2.0f) + l::math::sin(l::math::constants::PI_f * phaseMod * 2.0f));
    }

    /*********************************************************************/

    void SignalGeneratorSaw2::ResetInput() {
        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInput(mNumDefaultInputs + 1, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(mNumDefaultInputs + 1, InputBound::INPUT_0_TO_1);
        mUpdateRate = 16.0f;
    }

    void SignalGeneratorSaw2::Reset() {
        InitSaw(&mSaw, 0.0, 0.0);
    }

    void SignalGeneratorSaw2::UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {
        mAttenuation = mInputManager.GetValueNext(mNumDefaultInputs + 0);
        mCutoff = mInputManager.GetValueNext(mNumDefaultInputs + 1);
        mInputManager.SetUpdateRate(mNumDefaultInputs + 0, mUpdateRate);
        mInputManager.SetUpdateRate(mNumDefaultInputs + 1, mUpdateRate);
        mInputManager.SetDuration(mNumDefaultInputs + 0, 1.0f);
        mInputManager.SetDuration(mNumDefaultInputs + 1, 1.0f);

        UpdateSaw(&mSaw, 0.00001f + 0.99999f * mAttenuation * mAttenuation * mAttenuation, mCutoff * mCutoff);
    }

    float SignalGeneratorSaw2::ProcessSignal(float deltaTime, float freq) {
        return static_cast<float>(ProcessSaw(&mSaw, static_cast<double>(deltaTime) * static_cast<double>(freq)));
    }

    /*********************************************************************/
    void SignalGeneratorSine::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;

        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::max2(static_cast<double>(inputs.at(0).Get()), 0.0);
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

                //double limitFmMod = 1.0 / l::math::max(mFreq / 25.0, 1.0);
                //mPmod = 800.0 * fmMod * fmMod * limitFmMod;

                //double limitFmMod = 1.0 / l::math::max(mFreq / 25.0, 1.0);
                //mFmod = 800.0 * mFmod * mFmod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta = mDeltaTime * mFreq;

                    mPhaseFmod += phaseDelta;
                    mPhaseFmod = l::math::mod(mPhaseFmod, 1.0);
                    double modulation = mFmod * (1.0 + 0.5 * l::math::sin(l::math::constants::PI * mPhaseFmod * 2.0));

                    mPhase += phaseDelta * modulation;
                    mPhase = l::math::mod(mPhase, 1.0);

                    double phaseMod = mPhase + mPmod;
                    phaseMod = l::math::mod(phaseMod, 1.0);

                    mWave += smooth * (mPhase + phaseMod - mWave);
                    double sine = l::math::sin(l::math::constants::PI * mWave);

                    /*
                    double phaseDelta = mDeltaTime * mFreq;
                    mPhase += phaseDelta * mFmod;
                    mPhase = l::math::mod(mPhase, 1.0);

                    double phaseMod = mPhase + mPmod;
                    phaseMod = l::math::mod(phaseMod, 1.0);

                    double sine = l::math::sin(l::math::constants::PI * (mPhase + phaseMod));
                    */

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(sine);
                }
            }
        );
    }

    /*********************************************************************/
    void SignalGeneratorSineFM::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::max2(static_cast<double>(inputs.at(0).Get()), 0.0);
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
                    mPhaseFmod = l::math::mod(mPhaseFmod, 1.0);
                    double modWave = fmodGain * l::math::sin(l::math::constants::PI * mPhaseFmod * 0.5);
                    double fmod = (mFmodOfs + 1.0) * mFmodVol + mFmodVol * modWave;
                    fmod = l::math::clamp(fmod, 0.0, 500.0);

                    double phaseDelta = mDeltaTime * mFreq * (fmod + 1.0) * (mFmod + 1.0);
                    phaseDelta = l::math::clamp(phaseDelta, phaseDelta2, 0.5);
                    mPhase += phaseDelta;
                    mPhase = l::math::mod(mPhase, 1.0);

                    mWave += smooth * (mPhase - mWave);
                    double out = l::math::sin(l::math::constants::PI * mWave * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(out);
                }
            }
        );
    }

    /*********************************************************************/
    void SignalGeneratorSineFM2::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::max2(static_cast<double>(inputs.at(0).Get()), 0.0);
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

                double limitFmMod = 1.0 / l::math::max2(mFreq / 25.0, 1.0);
                fmMod = 800.0 * fmMod * fmMod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double maxFmModulation = 1.0 / l::math::max2(mFreq * fmFreq * mDeltaLimit, 1.0);
                    maxFmModulation *= maxFmModulation;
                    maxFmModulation *= maxFmModulation;
                    fmFreq = fmFreq * maxFmModulation;

                    double fmNote = mFreq * fmFreq;
                    double phaseDelta2 = mDeltaTime * fmNote;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::mod(mPhaseFmod, 1.0);
                    mFmod = (fmMod + fmMod * l::math::sin(l::math::constants::PI * mPhaseFmod * 2.0));

                    double phaseDelta = mDeltaTime * mFreq * (mFmod + 1.0) / (fmMod + 1.0);

                    mPhase += phaseDelta;
                    mPhase = l::math::mod(mPhase, 1.0);
                    mWave += smooth * (mPhase - mWave);

                    double out = l::math::sin(l::math::constants::PI * mWave * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(out);
                }
            }
        );
    }

    /*********************************************************************/
    void SignalGeneratorSineFM3::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::max2(static_cast<double>(inputs.at(0).Get()), 0.0);
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
                double limitFmMod = 1.0 / l::math::max2(mFreq / 25.0, 1.0);
                double fm = 800.0 * fmMod * fmMod * limitFmMod;

                for (int32_t i = start; i < end; i++) {
                    double phaseDelta2 = mDeltaTime * mFreq;
                    mPhaseFmod += phaseDelta2;
                    mPhaseFmod = l::math::mod(mPhaseFmod, 1.0);
                    double modulation = fm * l::math::sin(l::math::constants::PI * mPhaseFmod * 2.0);

                    double phaseDelta = mDeltaTime * mFreq * modulation;
                    mPhase += phaseDelta;
                    mPhase = l::math::mod(mPhase, 1.0);

                    mWave += smooth * (mPhase - mWave);
                    double out = l::math::sin(l::math::constants::PI * mWave * 2.0);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(out);
                }
            }
        );
    }

    /*********************************************************************/
    void SignalGeneratorSaw::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).Get(numSamples);

        float updateRate = 256.0f;
        mSamplesUntilUpdate = l::audio::BatchUpdate(updateRate, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::max2(static_cast<double>(inputs.at(0).Get()), 0.0);
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
                    mPhaseFmod = l::math::mod(mPhaseFmod, 1.0);
                    double modulation = mFmod * mPhaseFmod;

                    mPhase += phaseDelta * modulation;
                    mPhase = l::math::mod(mPhase, 1.0);

                    double phaseMod = mPhase + mPmod;
                    phaseMod = l::math::mod(phaseMod, 1.0);

                    double saw = mPhase + phaseMod;
                    mWave += smooth * (saw - mWave);

                    mVol += (1.0f / 256.0f) * (mVolume - mVol);

                    *output0++ = mVol * static_cast<float>(mWave - 1.0);
                }
            }
        );
    }
}
