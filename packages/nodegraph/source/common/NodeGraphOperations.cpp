#include "nodegraph/NodeGraphOperations.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    void GraphSourceConstants::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (int8_t i = 0; i < mNumOutputs; i++) {
            float val = inputs.at(i).Get();
            val = val > mMax ? mMax : val < mMin ? mMin : val;
            inputs.at(i).mInput.mInputFloatConstant = val;
            outputs.at(i).mOutput = val;
        }
    }

    void GraphSourceConstants::Tick(float, float) {
        mNode->ProcessSubGraph();
    }

    std::string_view GraphSourceConstants::GetName() {
        switch (mMode) {
        case 0:
            return "Constant [0,1]";
        case 1:
            return "Constant [-1,1]";
        case 2:
            return "Constant [0,100]";
        case 3:
            return "Constant [-inf,inf]";
        };
        return "";
    }

    bool GraphSourceConstants::IsDataVisible(int8_t) {
        return true;
    }

    bool GraphSourceConstants::IsDataEditable(int8_t) {
        return true;
    }

    void GraphSourceTime::Process(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>& outputs) {
        float rate = 44100.0f;
        float phaseChange = 1.0f / rate;
        mAudioTime += phaseChange;

        outputs.at(0).mOutput = mAudioTime;
        outputs.at(1).mOutput = mFrameTime;
    }

    void GraphSourceTime::Tick(float time, float) {
        mFrameTime = time;
    }

    void GraphSourceTime::Reset() {
        mAudioTime = 0.0f;
        mFrameTime = 0.0f;
    }

    std::string_view GraphSourceTime::GetOutputName(int8_t outputChannel) {
        return defaultOutStrings[outputChannel];
    }

    std::string_view GraphSourceTime::GetName() {
        return "Time";
    }

    void GraphSourceSine::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        //float time = inputs.at(0).Get();
        float freq = inputs.at(1).Get();
        float fMod = 1.0f + inputs.at(2).Get();
        float pMod = inputs.at(3).Get();
        float reset = inputs.at(4).Get();

        if (freq == 0.0f) {
            Reset();
            outputs.at(0).mOutput = 0.0f;
            return;
        }
        if (reset > 0.5f) {
            Reset();
        }

        float deltaTime = 1.0f / 44100.0f;
        float phaseDelta = deltaTime * freq;

        mPhase += phaseDelta * fMod;
        mPhase -= floorf(mPhase);

        float phaseMod = mPhase + pMod;
        phaseMod -= floorf(phaseMod);

        outputs.at(0).mOutput = sinf(3.141529f * (mPhase + phaseMod));
        outputs.at(1).mOutput = mPhase;
        outputs.at(2).mOutput = phaseMod;
    }

    void GraphSourceSine::Reset() {
        mPhase = 0.0f;
    }

    std::string_view GraphSourceSine::GetInputName(int8_t inputChannel) {
        return defaultInStrings[inputChannel];
    }

    std::string_view GraphSourceSine::GetOutputName(int8_t outputChannel) {
        return defaultOutStrings[outputChannel];
    }

    std::string_view GraphSourceSine::GetName() {
        return "Sine";
    }

    void GraphSourceKeyboard::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size();i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphSourceKeyboard::Tick(float, float) {
        mKeyboard.Update();
    }

    void GraphSourceKeyboard::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
    }

    std::string_view GraphSourceKeyboard::GetOutputName(int8_t outputChannel) {
        return defaultOutStrings[outputChannel];
    }

    std::string_view GraphSourceKeyboard::GetName() {
        return "Keyboard";
    }

    bool GraphSourceKeyboard::IsDataVisible(int8_t) {
        return true;
    }

    void GraphSourceKeyboard::NoteOn(int32_t note) {
        float frequency = l::audio::GetFrequencyFromNote(static_cast<float>(note));
        int8_t channel = GetNextNoteChannel(note);
        mNode->SetInput(static_cast<int8_t>(channel), frequency);
    }
    void GraphSourceKeyboard::NoteOff() {
        Reset();
    }

    void GraphSourceKeyboard::NoteOff(int32_t note) {
        int8_t channel = ResetNoteChannel(note);
        if (channel >= 0) {
            mNode->SetInput(channel, 0.0f);
        }
    }

    int8_t GraphSourceKeyboard::ResetNoteChannel(int32_t note) {
        for (size_t i = 0; i < mChannel.size(); i++) {
            if (mChannel.at(i).first == note) {
                mChannel.at(i).second = 0;
                return static_cast<int8_t>(i);
            }
        }
        // It is possible to get a note off for a note not playing because the channel was taken for another newer note
        return -1;
    }

    int8_t GraphSourceKeyboard::GetNextNoteChannel(int32_t note) {
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


    void GraphNumericAdd::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get() + inputs.at(1).Get();
    }

    void GraphNumericMultiply::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get();
    }

    void GraphNumericSubtract::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get() - inputs.at(1).Get();
    }

    void GraphNumericNegate::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = -inputs.at(0).Get();
    }

    void GraphNumericIntegral::Reset() {
        mOutput = 0.0f;
    }

    void GraphNumericIntegral::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mOutput += inputs.at(0).Get();
        outputs.at(0).mOutput = mOutput;
    }

    /* Logical operations */

    void GraphLogicalAnd::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 && input2) ? 1.0f : 0.0f;
    }

    void GraphLogicalOr::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 || input2) ? 1.0f : 0.0f;
    }

    void GraphLogicalXor::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 ^ input2) ? 1.0f : 0.0f;
    }

    /* Stateful filtering operations */

    void GraphFilterLowpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    void GraphFilterLowpass::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float cutoff = inputs.at(0).Get();
        float resonance = inputs.at(1).Get();
        float inputValue = inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = resonance * cutoff;

        mState0 = rc * mState0 - cutoff * (mState1 + inputValue);
        mState1 = rc * mState1 + cutoff * mState0;

        outputs.at(0).mOutput = -mState1;
    }

    void GraphFilterEnvelope::Reset() {
        mEnvelope = 0.0f;
    }

    void GraphFilterEnvelope::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float noteTarget = inputs.at(0).Get();
        float attackFrames = inputs.at(1).Get() * 44100.0f / 1000.0f;
        float releaseFrames = inputs.at(2).Get() * 44100.0f / 1000.0f;
        float noteFade = inputs.at(3).Get();

        if (noteTarget != 0.0f && mFrameCount < attackFrames) {
            if (mFrameCount == 0) {
                mNote = noteTarget;
            }
            // attack
            mNote = noteTarget;
            mFrameCount++;
        }

        if (noteTarget != 0.0f) {
            if (mEnvelopeTarget < 1.0f) {
                mEnvelopeTarget += 1.0f / attackFrames;
            }
            else {
                mEnvelopeTarget = 1.0f;
            }
            mNote += noteFade * noteFade * (noteTarget - mNote);
        }
        else {
            // release
            if (mFrameCount > 0) {
                mEnvelopeTarget -= 1.0f / releaseFrames;
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

    void GraphOutputSpeaker::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        auto& buffer = mAudioStream->GetWriteBuffer();
        buffer[mCurrentStereoPosition++] = inputs.at(0).Get();
        buffer[mCurrentStereoPosition++] = inputs.at(1).Get();
        mCurrentStereoPosition %= mAudioStream->GetPartTotalSize();
    }

    void GraphOutputSpeaker::Tick(float, float) {
    }

    void GraphEffectReverb1::Process(std::vector<NodeGraphInput>&inputs, std::vector<NodeGraphOutput>&outputs) {
        float wet = inputs.at(2).Get();
        
        fb = 0.33f * min(1.0f - inputs.at(3).Get(), 0.0f);

        float roomSize = inputs.at(4).Get();

        if (roomSize > maxRoomSizeInMeters) {
            roomSize = maxRoomSizeInMeters;
            mNode->SetInput(3, maxRoomSizeInMeters);
        }
        else if (roomSize < 0.1f) {
            roomSize = 0.1f;
            mNode->SetInput(3, 0.1f);
        }

        uint32_t bufSizeLimit = GetFramesPerRoomSize(roomSize);

        d0 = inputs.at(5).Get();
        fb0 = 0.5f * 0.45f * max(inputs.at(6).Get(), 1);
        d1 = inputs.at(7).Get();
        fb1 = 0.5f * 0.45f * max(inputs.at(8).Get(), 1);
        d2 = inputs.at(9).Get();
        fb2 = 0.5f * 0.45f * max(inputs.at(10).Get(), 1);

        float dry = 1.0f - wet;

        uint32_t delay0 = (int(bufIndex + d0 * bufSizeLimit)) % bufSizeLimit;
        uint32_t delay1 = (int(bufIndex + d1 * bufSizeLimit)) % bufSizeLimit;
        uint32_t delay2 = (int(bufIndex + d2 * bufSizeLimit)) % bufSizeLimit;
        float in0 = inputs.at(0).Get();
        float in1 = inputs.at(1).Get();

        float out0 = fb0 * buf0[delay0] + fb1 * buf1[delay1] + fb2 * buf0[delay2];
        outputs[0].mOutput = in0 * dry + out0 * wet;
        buf0[bufIndex] = (fb)*buf1[bufIndex] + in0 - out0;

        float out1 = fb0 * buf1[delay0] + fb1 * buf0[delay1] + fb2 * buf1[delay2];
        outputs[1].mOutput = in1 * dry + out1 * wet;
        buf1[bufIndex] = (fb)*buf0[bufIndex] + in1 - out1;

        bufIndex = (bufIndex + 1) % bufSizeLimit;

        delay0 = (delay0 + 1) % bufSizeLimit;
        delay1 = (delay1 + 1) % bufSizeLimit;
        delay2 = (delay2 + 1) % bufSizeLimit;
    }

    void GraphEffectReverb1::Tick(float, float) {
        if (!mInited) {
            mNode->SetInput(2, 0.75f);
            mNode->SetInput(3, 0.5f);
            mNode->SetInput(4, 30.0f);
            mNode->SetInput(5, 0.5f);
            mNode->SetInput(6, 0.9f);
            mNode->SetInput(7, 0.8f);
            mNode->SetInput(8, 0.9f);
            mNode->SetInput(9, 0.7f);
            mNode->SetInput(10, 0.9f);
            mInited = true;
        }
    }

    void GraphEffectReverb2::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float wet = inputs.at(2).Get();
        float reverbFeedback = min(inputs.at(3).Get(), 1.0f);
        float roomSize = inputs.at(4).Get();
        float stereoWidth = min(inputs.at(5).Get(), 1.0f);
        float earliestDelay = min(inputs.at(6).Get(), 5.0f);
        float longestDelay = min(inputs.at(7).Get(), 5.0f);
        float numDelays = max(inputs.at(8).Get(), 1.0f);
        float tapBulge = 1.0f + 9.0f * min(inputs.at(9).Get(), 1.0f);
        //float cutoff = min(inputs.at(10).Get(), 1.0f);
        //float res = min(inputs.at(11).Get(), 1.0f);

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
        /*
        cutoff *= cutoff;
        float rc = 1.0f - res * cutoff;

        mLP0 = rc * mLP0 - cutoff * (mLP1 + reverb0);
        mLP1 = rc * mLP1 + cutoff * mLP0;
        mLP2 = rc * mLP2 - cutoff * (mLP3 + reverb1);
        mLP3 = rc * mLP3 + cutoff * mLP2;
        float lp0 = mLP1;
        float lp1 = mLP3;
        */
        bufRev0[bufIndex] = reverbFeedback * (bufRev0[bufIndex] + reverb0);
        bufRev1[bufIndex] = reverbFeedback * (bufRev1[bufIndex] + reverb1);

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

    void GraphEffectReverb2::Tick(float, float) {
        if (!mInited) {
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

            /*
        float wet = inputs.at(2).Get();
        float reverbFeedback = min(inputs.at(3).Get(), 1.0f);
        float roomSize = inputs.at(4).Get();
        float stereoWidth = min(inputs.at(5).Get(), 1.0f);
        float earliestDelay = min(inputs.at(6).Get(), 1.0f);
        float longestDelay = min(inputs.at(7).Get(), 1.0f);
        float numDelays = max(inputs.at(8).Get(), 1.0f);
        float tapBulge = 1.0f + 9.0f * min(inputs.at(9).Get(), 1.0f);
        float cutoff = min(inputs.at(10).Get(), 1.0f);
        float res = min(inputs.at(11).Get(), 1.0f);
            */
            mInited = true;
        }
    }


}
