#pragma once
#include "nodegraph/NodeGraph.h"

#include "logging/LoggingAll.h"

#include "hid/KeyboardPiano.h"
#include "hid/Midi.h"

#include "audio/PortAudio.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <math.h>
#include <random>
#include <unordered_set>

namespace l::nodegraph {

    /*********************************************************************/
    class GraphInputKeyboardPiano : public NodeGraphOp, public l::hid::INoteProcessor {
    public:
        GraphInputKeyboardPiano(NodeGraphBase* node, l::hid::KeyState* keyState) :
            NodeGraphOp(node, 0, 3, 3)
        {
            mChannel.resize(1);
            mKeyboard.SetKeyState(keyState);
            mKeyboard.SetNoteProcessor(this);
        }

        std::string defaultOutStrings[3] = { "Freq", "Note On Id", "Note Off Id" };

        virtual ~GraphInputKeyboardPiano() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tickCount, float elapsed) override;
        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Keyboard";
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        virtual void NoteOn(int32_t note) override;
        virtual void NoteOff() override;
        virtual void NoteOff(int32_t note) override;
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        int8_t mNoteCounter = 0;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
        l::hid::KeyboardPiano mKeyboard;
    };

    /*********************************************************************/
    class GraphInputMidiKeyboard : public NodeGraphOp {
    public:
        GraphInputMidiKeyboard(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, 0, 4, 4),
            mMidiManager(midiManager)
        {
            mChannel.resize(1);

            mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                MidiEvent(data);
                });
        }

        std::string defaultOutStrings[4] = { "Freq", "Velocity", "Note On Id", "Note Off Id"};

        virtual ~GraphInputMidiKeyboard() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Midi Device";
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
        void NoteOn(int32_t note, int32_t velocity);
        void NoteOff();
        void NoteOff(int32_t note);
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        l::hid::midi::MidiManager* mMidiManager = nullptr;

        int8_t mNoteCounter = 0;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
    };

    /*********************************************************************/
    class GraphInputMidiKnobs : public NodeGraphOp {
    public:
        GraphInputMidiKnobs(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, 0, 8, 8),
            mMidiManager(midiManager)
        {
            mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                MidiEvent(data);
                });
        }

        std::string defaultOutStrings[8] = { "Knob 1", "Knob 2", "Knob 3", "Knob 4", "Knob 5", "Knob 6", "Knob 7", "Knob 8", };

        virtual ~GraphInputMidiKnobs() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Reset() override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Midi Knobs";
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        l::hid::midi::MidiManager* mMidiManager = nullptr;
    };

    /*********************************************************************/

    enum class MidiButtonStates {
        DISCONNECTED = 0, // non lit
        ALLOCATED, // yellow blinking (in a node button group)
        CONNECTED, // yellow (output polled)
        OFF, // red, off - 0
        HALF_OFF, // red blinking - 0.33
        HALF_ON, // green blinking - 0.67
        ON, // green - 1
    };

    class GraphInputMidiButtons : public NodeGraphOp {
    public:

        // midi button commands
        // 0 - not lit
        // 1 - green
        // 2 - green flashing
        // 3 - red
        // 4 - red flashing
        // 5 - yellow
        // 6 - yellow flashing

        // remap to
        const int8_t BUTTON_DISCONNECTED = 0;  // non lit          - undefined
        const int8_t BUTTON_ALLOCATED = 1;     // yellow flashing  - undefined
        const int8_t BUTTON_OFF = 2;           // red              - 0.00f
        const int8_t BUTTON_ON_AND_OFF = 3;    // yellow           - 0.50f
        const int8_t BUTTON_ON = 4;            // green            - 1.00f

        const std::vector<int8_t> remapToButtonStatesToColor = {0, 6, 3, 5, 1, 4, 2};

        GraphInputMidiButtons(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager, int32_t buttonGroup) :
            NodeGraphOp(node, 0, 8, 8),
            mMidiManager(midiManager),
            mButtonGroup(buttonGroup)
        {
            mNodeName = "Midi Button Group ";
            mNodeName += std::to_string(mButtonGroup);
            mButtonStates.resize(8);

            for (int8_t i = 0; i < 8; i++) {
                defaultOutStrings[i] = std::string("Pad ") + std::to_string(i);
            }

            if (mMidiManager) {
                mCallbackId = mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                    MidiEvent(data);
                    });

                // turn all buttons on
                for (int8_t i = 0; i < 8; i++) {
                    UpdateButton(i, BUTTON_ALLOCATED);
                }
            }
        }

        std::string defaultOutStrings[8];

        virtual ~GraphInputMidiButtons() {
            // turn off all buttons before destructing
            for (int8_t i = 0; i < 8; i++) {
                UpdateButton(i, BUTTON_DISCONNECTED);
            }

            std::this_thread::yield();

            mMidiManager->UnregisterCallback(mCallbackId);
        }
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tick, float deltaTime) override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return mNodeName;
        }
        virtual bool IsDataVisible(int8_t) override {
            return true;
        }
        void UpdateButton(int8_t buttonId, int8_t buttonState) {
            if (mMidiManager) {
                int8_t buttonColor = remapToButtonStatesToColor.at(buttonState);
                mMidiManager->SendToDevice(0, 0x90, 0, mButtonGroup * 8 + buttonId, buttonColor);
                mButtonStates.at(buttonId) = buttonState;
            }
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mCallbackId = 0;
        int32_t mButtonGroup;
        std::string mNodeName;
        bool mMidiShiftState = false;
        std::vector<int8_t> mButtonStates;
        int32_t mInitCounter = 0;
    };
}

