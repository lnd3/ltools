#include "nodegraph/NodeGraphOperations.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    void GraphSourceConstants::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void GraphSourceTime::ProcessSubGraph(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>& outputs) {
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

    void GraphSourceSine::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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

    void GraphSourceKeyboard::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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


    void GraphNumericAdd::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get() + inputs.at(1).Get();
    }

    void GraphNumericMultiply::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get();
    }

    void GraphNumericSubtract::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get() - inputs.at(1).Get();
    }

    void GraphNumericNegate::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = -inputs.at(0).Get();
    }

    void GraphNumericIntegral::Reset() {
        mOutput = 0.0f;
    }

    void GraphNumericIntegral::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mOutput += inputs.at(0).Get();
        outputs.at(0).mOutput = mOutput;
    }

    /* Logical operations */

    void GraphLogicalAnd::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 && input2) ? 1.0f : 0.0f;
    }

    void GraphLogicalOr::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 || input2) ? 1.0f : 0.0f;
    }

    void GraphLogicalXor::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 ^ input2) ? 1.0f : 0.0f;
    }

    /* Stateful filtering operations */

    void GraphFilterLowpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    void GraphFilterLowpass::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float cutoff = inputs.at(0).Get();
        float resonance = inputs.at(1).Get();
        float inputValue = inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = 1 - resonance * cutoff;

        mState0 = rc * mState0 - cutoff * (mState1 + inputValue);
        mState1 = rc * mState1 + cutoff * mState0;

        outputs.at(0).mOutput = -mState1;
    }

    void GraphOutputSpeaker::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>&) {
        auto& buffer = mAudioStream->GetWriteBuffer();
        buffer[mCurrentStereoPosition++] = inputs.at(0).Get();
        buffer[mCurrentStereoPosition++] = inputs.at(1).Get();
        mCurrentStereoPosition %= mAudioStream->GetPartTotalSize();
    }

    void GraphOutputSpeaker::Tick(float, float) {
    }

    void GraphEffectReverb::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {

        // 0 - in 1
        // 1 - in 2
        //node->SetInput(2, 0.1f); // mix
        //node->SetInput(3, 0.3f); // attenuation
        //node->SetInput(4, 30.0f); // room size

        //node->SetInput(5, 0.2f); // delay 1
        //node->SetInput(6, 0.5f); // feedback 1

        //node->SetInput(7, 2.3f); // delay 2
        //node->SetInput(8, 0.79f); // feedback 2

        //node->SetInput(9, 0.5f); // delay 3
        //node->SetInput(10, 0.96f); // feedback 3

        float wet = inputs.at(2).Get();
        
        fb = 0.33f * min(1.0f - inputs.at(3).Get() , 0.0f);
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

        // feedback and delay
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

}
