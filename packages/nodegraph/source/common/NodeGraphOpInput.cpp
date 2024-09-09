#include "nodegraph/NodeGraphOpInput.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /*********************************************************************/
    void GraphInputKeyboardPiano::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
        mNode->SetInput(1, static_cast<float>(l::audio::gNoNote_f));
        mNode->SetInput(2, static_cast<float>(l::audio::gNoNote_f));
    }

    void GraphInputKeyboardPiano::Tick(int32_t, float) {
        mKeyboard.Update();
    }

    void GraphInputKeyboardPiano::Reset() {
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, l::audio::gNoNote_f);
        mNode->SetInput(2, l::audio::gNoNote_f);
    }

    void GraphInputKeyboardPiano::NoteOn(int32_t note) {
        float frequency = l::audio::GetFrequencyFromNote(static_cast<float>(note));
        mNode->SetInput(0, frequency);
        mNode->SetInput(1, static_cast<float>(note));
    }
    void GraphInputKeyboardPiano::NoteOff() {
        Reset();
    }

    void GraphInputKeyboardPiano::NoteOff(int32_t note) {
        mNode->SetInput(2, static_cast<float>(note));
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
    }

    void GraphInputMidiKeyboard::Reset() {
        mNode->SetInput(0, 0.0f);
        mNode->SetInput(1, 0.0f);
        mNode->SetInput(2, l::audio::gNoNote_f, 8);
        mNode->SetInput(3, l::audio::gNoNote_f, 8);
    }

    void GraphInputMidiKeyboard::MidiEvent(const l::hid::midi::MidiData& data) {
        //LOG(LogInfo) << "listener 1: dev" << data.device << " stat " << data.status << " ch " << data.channel << " d1 " << data.data1 << " d2 " << data.data2;
        if (data.channel != 1) {
            return;
        }
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
        mNode->SetInput(0, frequency);
        mNode->SetInput(1, velocity / 128.0f);

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
        Reset();
    }

    void GraphInputMidiKeyboard::NoteOff(int32_t note) {
        auto input3 = &mNode->GetInput(3, 8);
        for (int32_t i = 0; i < 8; i++) {
            if (l::math::functions::equal(*input3, l::audio::gNoNote_f)) {
                *input3 = static_cast<float>(note);
                break;
            }
            input3++;
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
        if (data.channel != 0) {
            return;
        }
        if (data.status == 11) {
            if (data.data1 >= 48 && data.data1 <= 55) {
                mNode->SetInput(static_cast<int8_t>(data.data1 - 48), data.data2 / 128.0f);
            }
        }
    }

    /*********************************************************************/
    void GraphInputMidiButtons::Reset() {
        for (int8_t i = 0; i < GetNumInputs(); i++) {
            mNode->SetInput(i, 0.0f);
        }
    }

    void GraphInputMidiButtons::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphInputMidiButtons::Tick(int32_t, float) {
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
        if (data.channel != 0) {
            return;
        }
        if (data.data1 == 98) {
            // shift button event
            mMidiShiftState = data.status == 9 ? true : data.status == 8 ? false : mMidiShiftState;
        }

        int8_t buttonIndex = static_cast<int8_t>(data.data1 - mButtonGroup * 8);
        if (buttonIndex >= 0 && buttonIndex < 8) {
            // pad button event
            if (data.status == 9) {
                if (mButtonStates.at(buttonIndex) <= BUTTON_ALLOCATED) {
                    // if button has not been polled before user input, set button to off first time
                    mButtonStates.at(buttonIndex) = BUTTON_OFF;
                }else if (!mMidiShiftState && mButtonStates.at(buttonIndex) < BUTTON_ON) {
                    mButtonStates.at(buttonIndex)++;
                }else if (mMidiShiftState && mButtonStates.at(buttonIndex) > BUTTON_OFF) {
                    mButtonStates.at(buttonIndex)--;
                }
            }
            float outputValue = static_cast<float>(mButtonStates.at(buttonIndex) - BUTTON_OFF) / 2.0f;
            mNode->SetInput(buttonIndex, outputValue);
            UpdateButton(buttonIndex, mButtonStates.at(buttonIndex));
        }
    }
}
