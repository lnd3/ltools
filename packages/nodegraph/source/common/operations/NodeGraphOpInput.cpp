#include "nodegraph/operations/NodeGraphOpInput.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphInputKeyboardPiano::Reset() {
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, l::audio::gNoNote_f, 8);
        mNode->SetInput(2, l::audio::gNoNote_f, 8);

        mNode->GetOutput(1, 8);
        mNode->GetOutput(2, 8);
    }

    void GraphInputKeyboardPiano::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get();

        auto output1 = &outputs.at(1).GetOutput(8);
        auto output2 = &outputs.at(2).GetOutput(8);
        auto input1 = &inputs.at(1).Get(8);
        auto input2 = &inputs.at(2).Get(8);
        for (int32_t i = 0; i < 8; i++) {
            *output1++ = *input1;
            *output2++ = *input2;
            *input1++ = l::audio::gNoNote_f;
            *input2++ = l::audio::gNoNote_f;
        }
    }

    void GraphInputKeyboardPiano::Tick(int32_t, float) {
        mKeyboard.Update();
    }

    void GraphInputKeyboardPiano::NoteOn(int32_t note, int32_t) {
        mLastNote = note;
        float frequency = l::audio::GetFrequencyFromNote(static_cast<float>(note));
        mNode->SetInput(0, frequency);

        auto input1 = &mNode->GetInput(1, 8);
        for (int32_t i = 0; i < 8; i++) {
            if (l::math::functions::equal(*input1, l::audio::gNoNote_f)) {
                *input1 = static_cast<float>(note);
                break;
            }
            input1++;
        }
    }
    void GraphInputKeyboardPiano::NoteOff() {
        Reset();
    }

    void GraphInputKeyboardPiano::NoteOff(int32_t note) {
        if (mLastNote == note) {
            mNode->SetInput(0, 0.0f);
        }
        auto input2 = &mNode->GetInput(2, 8);
        for (int32_t i = 0; i < 8; i++) {
            if (l::math::functions::equal(*input2, l::audio::gNoNote_f)) {
                *input2 = static_cast<float>(note);
                break;
            }
            input2++;
        }
    }

    void GraphInputKeyboardPiano::NoteSustain(bool) {
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
    void GraphInputMidiKeyboard::Reset() {
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.0f);
        mNode->SetInput(2, l::audio::gNoNote_f, 8);
        mNode->SetInput(3, l::audio::gNoNote_f, 8);
        mNode->SetInput(4, 0.0f);

        mNode->GetOutput(2, 8);
        mNode->GetOutput(3, 8);
    }

    void GraphInputMidiKeyboard::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        outputs.at(0).mOutput = inputs.at(0).Get();
        outputs.at(1).mOutput = inputs.at(1).Get();

        auto output2 = &outputs.at(2).GetOutput(8);
        auto output3 = &outputs.at(3).GetOutput(8);
        auto input2 = &inputs.at(2).Get(8);
        auto input3 = &inputs.at(3).Get(8);
        for (int32_t i = 0; i < 8; i++) {
            *output2++ = *input2;
            *output3++ = *input3;
            *input2++ = l::audio::gNoNote_f;
            *input3++ = l::audio::gNoNote_f;
        }
        outputs.at(4).mOutput = l::math::functions::round(inputs.at(4).Get());
    }

    void GraphInputMidiKeyboard::Tick(int32_t, float) {
        auto deviceInId = static_cast<uint32_t>(mNode->GetInput(4, 1) + 0.5f);
        SetDevice(deviceInId);
    }

    void GraphInputMidiKeyboard::MidiEvent(const l::hid::midi::MidiData& data) {
        //LOG(LogInfo) << "listener 1: dev" << data.device << " stat " << data.status << " ch " << data.channel << " d1 " << data.data1 << " d2 " << data.data2;
        if (mMidiChannelKeys < 0 || data.channel != static_cast<uint32_t>(mMidiChannelKeys)) {
            return;
        }
        if (data.status == 9) {
            if (data.data2 > 0) {
                // note on
                NoteOn(data.data1, data.data2);
            }
            else {
                // note off
                NoteOff(data.data1);
            }
        }
        else if (data.status == 8) {
            // note off
            NoteOff(data.data1);
        }
        else if (data.status == 11) {
            if (data.data1 == 64) {
                // sustain
                NoteSustain(data.data2 != 0);
            }
        }
    }

    void GraphInputMidiKeyboard::NoteOn(int32_t note, int32_t velocity) {
        mLastNote = note;
        float frequency = l::audio::GetFrequencyFromNote(static_cast<float>(note));
        mNode->SetInput(0, frequency);
        mNode->SetInput(1, velocity / 128.0f);

        if (mSustain && !mSustainedNotes.empty()) {
            std::erase_if(mSustainedNotes, [&](const int32_t& sustainedNote) {
                return sustainedNote == note;
                });
        }
        auto input2 = &mNode->GetInput(2, 8);
        for (int32_t i = 0; i < 8; i++) {
            if (l::math::functions::equal(*input2, l::audio::gNoNote_f)) {
                *input2 = static_cast<float>(note);
                break;
            }
            input2++;
        }
    }
    void GraphInputMidiKeyboard::NoteOff() {
    }

    void GraphInputMidiKeyboard::NoteOff(int32_t note) {
        if (mLastNote == note) {
            mNode->SetInput(0, 0.0f);
        }

        if (!mSustain) {
            auto input3 = &mNode->GetInput(3, 8);
            for (int32_t i = 0; i < 8; i++) {
                if (l::math::functions::equal(*input3, l::audio::gNoNote_f)) {
                    *input3 = static_cast<float>(note);
                    break;
                }
                input3++;
            }
        }
        else {
            mSustainedNotes.push_back(note);
        }
    }

    void GraphInputMidiKeyboard::NoteSustain(bool on) {
        mSustain = on;
        if (!mSustain && !mSustainedNotes.empty()) {
            for (auto note : mSustainedNotes) {
                auto input3 = &mNode->GetInput(3, 8);
                for (int32_t i = 0; i < 8; i++) {
                    if (l::math::functions::equal(*input3, l::audio::gNoNote_f)) {
                        *input3 = static_cast<float>(note);
                        break;
                    }
                    input3++;
                }
            }
            mSustainedNotes.clear();
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
    void GraphInputMidiKnobs::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
        mNode->SetInput(8, 0.0f);
    }

    void GraphInputMidiKnobs::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphInputMidiKnobs::Tick(int32_t, float) {
        auto deviceInId = static_cast<uint32_t>(mNode->GetInput(8, 1) + 0.5f);
        SetDevice(deviceInId);
    }

    void GraphInputMidiKnobs::MidiEvent(const l::hid::midi::MidiData& data) {
        if (mMidiChannelKnobs < 0 || data.channel != static_cast<uint32_t>(mMidiChannelKnobs)) {
            return;
        }
        if (data.status == 11) {
            if (data.data1 >= 48 && data.data1 < 56) {
                mNode->SetInput(static_cast<int8_t>(data.data1 - 48), data.data2 / 128.0f);
            }
            else if (data.data1 >= 7 && data.data1 < 15) {
                mNode->SetInput(static_cast<int8_t>(data.data1 - 7), data.data2 / 128.0f);
            }
        }
    }


    /*********************************************************************/
    void GraphInputMidiButtons::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
        mNode->SetInput(8, 0.0f);
    }

    void GraphInputMidiButtons::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphInputMidiButtons::Tick(int32_t, float) {
        auto deviceInId = static_cast<uint32_t>(mNode->GetInput(8, 1) + 0.5f);
        SetDevice(deviceInId);

        if (mInitCounter == 120){
            for (int8_t i = 0; i < 8; i++) {
                if (mButtonStates.at(i) == BUTTON_ALLOCATED) {
                    // Set to off when allocated and output has been polled, i.e been connected and sampled
                    UpdateButton(i, BUTTON_OFF);
                }
            }
        }
        mInitCounter++;
    }

    void GraphInputMidiButtons::MidiEvent(const l::hid::midi::MidiData& data) {
        if (mMidiChannelButtons < 0 || data.channel != static_cast<uint32_t>(mMidiChannelButtons)) {
            return;
        }
        if (data.status == 9) {
            if (data.data1 >= 64 && data.data1 <= 67) {
                // navigation buttons up, down, left, right
                // button colors none, red
                UpdateCommonButton(data.data1, 1);
            }
            else if (data.data1 >= 68 && data.data1 <= 71) {
                // navigation buttons volume, pan, send, device
                // button colors none, red
                UpdateCommonButton(data.data1, 1);
            }
            else if (data.data1 >= 81 && data.data1 <= 86) {
                // navigation buttons stops all clips, clip stop, solo, rec arm, mute, select
                // button colors none, green
                UpdateCommonButton(data.data1, 1);
            }
            else if (data.data1 >= 91 && data.data1 <= 93) {
                // navigation buttons play/pause, rec
                // no buttons colors
            }
            else if (data.data1 == 98) {
                // shift button event
                // no buttons color
                mMidiShiftState = data.status == 9 ? true : data.status == 8 ? false : mMidiShiftState;
            }
            else {
                int8_t buttonIndex = static_cast<int8_t>(data.data1 - mButtonGroup * 8);
                if (buttonIndex >= 0 && buttonIndex < 8) {
                    // pad button event
                    if (mButtonStates.at(buttonIndex) <= BUTTON_ALLOCATED) {
                        // if button has not been polled before user input, set button to off first time
                        mButtonStates.at(buttonIndex) = BUTTON_OFF;
                    }
                    else if (!mMidiShiftState && mButtonStates.at(buttonIndex) < BUTTON_ON) {
                        mButtonStates.at(buttonIndex)++;
                    }
                    else if (mMidiShiftState && mButtonStates.at(buttonIndex) > BUTTON_OFF) {
                        mButtonStates.at(buttonIndex)--;
                    }
                    float outputValue = static_cast<float>(mButtonStates.at(buttonIndex) - BUTTON_OFF) / 2.0f;
                    mNode->SetInput(buttonIndex, outputValue);
                    UpdateButton(buttonIndex, mButtonStates.at(buttonIndex));
                }
            }
        }else if (data.status == 8) {
            if (data.data1 >= 64 && data.data1 <= 67) {
                // navigation buttons up, down, left, right
                UpdateCommonButton(data.data1, 0);
            }
            else if (data.data1 >= 68 && data.data1 <= 71) {
                // navigation buttons volume, pan, send, device
                UpdateCommonButton(data.data1, 0);
            }
            else if (data.data1 >= 81 && data.data1 <= 86) {
                // navigation buttons stops all clips, clip stop, solo, rec arm, mute, select
                UpdateCommonButton(data.data1, 0);
            }
            else if (data.data1 >= 91 && data.data1 <= 93) {
                // navigation buttons stops all clips, clip stop, solo, rec arm, mute, select
            }
            else if (data.data1 == 98) {
                // shift button event
                mMidiShiftState = data.status == 9 ? true : data.status == 8 ? false : mMidiShiftState;
            }
        }
        else if (data.status == 14) {
            // pb (fade wheel)

        }
    }
}
