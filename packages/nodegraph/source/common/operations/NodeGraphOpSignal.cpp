#include "nodegraph/operations/NodeGraphOpSignal.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphSignalBase::Reset() {
        mFreq = 0.0f;
        mVolumeTarget = 0.0f;
        mSmooth = 0.5f;
        mSignal = 0.0f;
        mWave = 0.0f;
        mDeltaPhase = 0.0f;
        mDeltaTime = 0.0f;
        mVolume = 0.0f;
        mSamplesUntilUpdate = 0.0f;
        mUpdateSamples = 16.0f;
        mHPCutoff = 0.5f;
        mHPResonance = 0.0001f;
        mHPState0 = 0.0f;
        mHPState1 = 0.0f;
        mHPCutoff = 0.0f;

        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.0f);
        mNode->SetInput(2, 0.5f);
        mNode->SetInput(3, 1.0f);
        mNode->SetInput(4, 0.0f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInput(6, 0.5f);
        mNode->SetInputBound(0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(1, InputBound::INPUT_CUSTOM, 0.0f, l::math::constants::FLTMAX);
        mNode->SetInputBound(2, InputBound::INPUT_0_100);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(6, InputBound::INPUT_0_TO_1);

        ResetSignal();
    }

    void GraphSignalBase::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(mUpdateSamples, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mDeltaTime = 1.0f / 44100.0f;
                mReset = inputs.at(0).Get();
                mFreq = inputs.at(1).Get();
                mVolumeTarget = inputs.at(2).Get();
                mSmooth = 0.5f * inputs.at(3).Get();
                float phaseExpansion = inputs.at(6).Get();

                mDeltaPhase = mDeltaTime * mFreq;
                //float deltaPhaseExpanded = l::math::functions::pow(mDeltaPhase, phaseExpansion);
                float deltaPhaseExpanded = l::math::functions::pow(mDeltaPhase, 0.4f - phaseExpansion * mDeltaPhase * 6.0f);
                mHPCutoff = deltaPhaseExpanded + (1.0f - deltaPhaseExpanded) * inputs.at(4).Get();
                mHPResonance = 1.0f - inputs.at(5).Get();

                if (mFreq == 0.0f || mReset > 0.5f) {
                    mVolumeTarget = 0.0f;
                }

                UpdateSignal(inputs, outputs);

            },
            [&](int32_t start, int32_t end, bool) {
                for (int32_t i = start; i < end; i++) {
                    float signalTarget = GenerateSignal(mDeltaTime, mFreq, mDeltaPhase);

                    // highpass filter
                    {
                        //float deltaPhase2 = l::math::functions::pow(mDeltaPhase, 0.125f);
                        float cutoff = mHPCutoff;
                        cutoff *= cutoff;
                        float rc = 1.0f - mHPResonance * cutoff;
                        float v01 = mHPState0 - mHPState1;
                        mHPState0 += cutoff * (signalTarget - mHPState0 + rc * v01);
                        mHPState1 += cutoff * v01;

                        signalTarget = signalTarget - mHPState1;
                    }

                    mSignal += mSmooth * (signalTarget - mSignal);
                    mVolume += (1.0f / 256.0f) * (mVolumeTarget - mVolume);
                    *output0++ = mVolume * mSignal;
                }
            }
        );
    }

    /*********************************************************************/

    void GraphSignalSine2::ResetSignal() {
        mNode->SetInput(mNumDefaultInputs + 0, 0.0f);
        mNode->SetInput(mNumDefaultInputs + 1, 0.0f);
        mNode->SetInputBound(mNumDefaultInputs + 0, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(mNumDefaultInputs + 1, InputBound::INPUT_0_TO_1);
        mUpdateSamples = 16.0f;
    }

    void GraphSignalSine2::UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        mFmod = inputs.at(mNumDefaultInputs + 0).Get();
        mPmod = inputs.at(mNumDefaultInputs + 1).Get();
        mFmod *= 0.25f * 0.25f * 0.5f * 44100.0f / l::math::functions::max(mFreq, 1.0f);
    }

    float GraphSignalSine2::GenerateSignal(float, float, float deltaPhase) {
        mPhaseFmod += deltaPhase;
        mPhaseFmod = l::math::functions::mod(mPhaseFmod, 1.0f);
        float modulation = l::math::functions::cos(l::math::constants::PI_f * mPhaseFmod * 2.0f);

        mPhase = mPhaseFmod;
        mPhase += mFmod * modulation;
        mPhase -= l::math::functions::floor(mPhase);

        float phaseMod = mPhaseFmod + mPmod;
        phaseMod -= l::math::functions::floor(phaseMod);

        //mWave = mSmooth * (mPhase + phaseMod - mWave);
        return 0.5f * (l::math::functions::sin(l::math::constants::PI_f * mPhase * 2.0f) + l::math::functions::sin(l::math::constants::PI_f * phaseMod * 2.0f));
    }

    /*********************************************************************/
    void GraphSignalSine::Reset() {
        // { "Freq Hz", "Freq Mod", "Phase Mod", "Reset"};
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.0f);
        mNode->SetInput(2, 1.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.5f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_CUSTOM, 0.0f, l::math::constants::FLTMAX);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_2);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
    }

    void GraphSignalSine::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(5).Get();

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
    void GraphSignalSineFM::Reset() {
        // { "Note", "Volume", "Fmod", "FmodFreq", "FmodVol", "FmodOfs", "Reset"}
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 0.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.0f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInput(6, 0.0f);
        mNode->SetInput(7, 0.5f);
        mNode->SetInput(8, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_CUSTOM, 0.0f, l::math::constants::FLTMAX);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(3, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(4, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(5, InputBound::INPUT_UNBOUNDED);
        mNode->SetInputBound(6, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(7, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(8, InputBound::INPUT_0_TO_1);
    }

    void GraphSignalSineFM::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(8).Get();

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
    void GraphSignalSineFM2::Reset() {
        // { "Note", "Volume", "FmodVol", "FmodOfs", "Reset"}
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 0.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.5f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_CUSTOM, 0.0f, l::math::constants::FLTMAX);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_100);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
    }

    void GraphSignalSineFM2::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(5).Get();

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
    void GraphSignalSineFM3::Reset() {
        // { "Note", "Volume", "Fmod", "Reset"}
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.5f);
        mNode->SetInput(2, 0.5f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_CUSTOM, 0.0f, l::math::constants::FLTMAX);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
    }

    void GraphSignalSineFM3::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(16.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(4).Get();

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
    void GraphSignalSaw::Reset() {
        // { "Freq", "Volume", "Fmod", "Phase", "Reset"};
        mPhase = 0.0f;
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.0f);
        mNode->SetInput(2, 1.0f);
        mNode->SetInput(3, 0.0f);
        mNode->SetInput(4, 0.5f);
        mNode->SetInput(5, 0.0f);
        mNode->SetInputBound(0, InputBound::INPUT_CUSTOM, 0.0f, l::math::constants::FLTMAX);
        mNode->SetInputBound(1, InputBound::INPUT_0_100);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_2);
        mNode->SetInputBound(3, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(4, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(5, InputBound::INPUT_0_TO_1);
    }

    void GraphSignalSaw::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float* output0 = &outputs.at(0).GetOutput(numSamples);

        mSamplesUntilUpdate = l::audio::BatchUpdate(256.0f, mSamplesUntilUpdate, 0, numSamples,
            [&]() {
                mFreq = l::math::functions::max(static_cast<double>(inputs.at(0).Get()), 0.0);
                mVolume = inputs.at(1).Get();
                mReset = inputs.at(5).Get();

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
