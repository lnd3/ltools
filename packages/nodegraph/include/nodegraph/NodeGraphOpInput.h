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
        virtual void NoteOn(int32_t note, int32_t velocity) override;
        virtual void NoteOff() override;
        virtual void NoteOff(int32_t note) override;
        virtual void NoteSustain(bool on) override;
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        int32_t mLastNote = 0;
        int8_t mNoteCounter = 0;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
        l::hid::KeyboardPiano mKeyboard;
    };

    /*********************************************************************/
    class GraphInputMidiKeyboard : public NodeGraphOp, public l::hid::INoteProcessor {
    public:
        GraphInputMidiKeyboard(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, 0, 5, 5),
            mMidiManager(midiManager)
        {
            mChannel.resize(1);

            SetDevice(0);

            mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                if (mMidiDeviceInId >= 0 && data.deviceIn == static_cast<uint32_t>(mMidiDeviceInId)) {
                    MidiEvent(data);
                }
                });
        }

        std::string defaultOutStrings[5] = { "Freq", "Velocity", "Note On Id", "Note Off Id", "Device Id"};

        virtual ~GraphInputMidiKeyboard() = default;
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
        virtual bool IsDataEditable(int8_t channel) override {
            return channel >= 4 ? true : false;
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
        virtual void NoteOn(int32_t note, int32_t velocity = 127) override;
        virtual void NoteOff() override;
        virtual void NoteOff(int32_t note) override;
        virtual void NoteSustain(bool on) override;
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        void SetDevice(int32_t deviceId) {
            if (deviceId >= 0 && mMidiDeviceInId != deviceId) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(static_cast<uint32_t>(deviceId));
                if (deviceInfo) {
                    mMidiDeviceInId = deviceId;
                    mMidiChannelKeys = deviceInfo->mChannelKeys;
                    mNodeName = deviceInfo->GetName();
                    mNodeName += " : Midi";
                }
            }
        }

        l::hid::midi::MidiManager* mMidiManager = nullptr;

        int32_t mLastNote = 0;
        int8_t mNoteCounter = 0;
        bool mSustain = false;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
        int32_t mMidiDeviceInId = -1;
        int32_t mMidiChannelKeys = -1;
        std::string mNodeName;
        std::vector<int32_t> mSustainedNotes;
    };

    /*********************************************************************/
    class GraphInputMidiKnobs : public NodeGraphOp {
    public:
        GraphInputMidiKnobs(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, 0, 9, 9),
            mMidiManager(midiManager)
        {
            SetDevice(0);

            mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                if (mMidiDeviceInId >= 0 && data.deviceIn == static_cast<uint32_t>(mMidiDeviceInId)) {
                    MidiEvent(data);
                }
                });
        }

        std::string defaultOutStrings[9] = { "Knob 1", "Knob 2", "Knob 3", "Knob 4", "Knob 5", "Knob 6", "Knob 7", "Knob 8", "Device Id"};

        virtual ~GraphInputMidiKnobs() = default;
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
        virtual bool IsDataEditable(int8_t channel) override {
            return channel >= 8 ? true : false;
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        void SetDevice(int32_t deviceId) {
            if (deviceId >= 0 && mMidiDeviceInId != deviceId) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(static_cast<uint32_t>(deviceId));
                if (deviceInfo) {
                    mMidiDeviceInId = deviceId;
                    mMidiChannelKnobs = deviceInfo->mChannelKnobs;
                    mNodeName = deviceInfo->GetName();
                    mNodeName += " : Knobs";
                }
            }
        }

        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mMidiDeviceInId = -1;
        int32_t mMidiChannelKnobs = -1;
        std::string mNodeName;
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
            NodeGraphOp(node, 0, 9, 9),
            mMidiManager(midiManager),
            mButtonGroup(buttonGroup)
        {
            mButtonStates.resize(8);

            for (int8_t i = 0; i < 8; i++) {
                defaultOutStrings[i] = std::string("Pad ") + std::to_string(i);
            }
            defaultOutStrings[8] = "Device Id";

            SetDevice(0);

            if (mMidiManager) {
                mCallbackId = mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                    if (mMidiDeviceInId >= 0 && data.deviceIn == static_cast<uint32_t>(mMidiDeviceInId)) {
                        MidiEvent(data);
                    }
                    });

                // turn all buttons on
                for (int8_t i = 0; i < 8; i++) {
                    UpdateButton(i, BUTTON_ALLOCATED);
                }
            }
        }

        std::string defaultOutStrings[9];

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
        virtual bool IsDataEditable(int8_t channel) override {
            return channel >= 8 ? true : false;
        }

        void UpdateButton(int8_t buttonId, int8_t buttonState) {
            if (mMidiManager) {
                int8_t buttonColor = remapToButtonStatesToColor.at(buttonState);
                auto deviceInfo = mMidiManager->GetDeviceInfo(mMidiDeviceInId);
                if (deviceInfo && deviceInfo->HasMidiOut()) {
                    mMidiManager->SendToDevice(deviceInfo->GetMidiOut(), 0x90, 0, mButtonGroup * 8 + buttonId, buttonColor);
                }
                mButtonStates.at(buttonId) = buttonState;
            }
        }

        void UpdateCommonButton(uint32_t buttonId, uint32_t buttonState) {
            if (mMidiManager) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(mMidiDeviceInId);
                if (deviceInfo) {
                    mMidiManager->SendToDevice(deviceInfo->GetMidiOut(), 0x90, 0, buttonId, buttonState);
                }
            }
        }

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        void SetDevice(int32_t deviceId) {
            if (deviceId >= 0 && mMidiDeviceInId != deviceId) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(static_cast<uint32_t>(deviceId));
                if (deviceInfo) {
                    mMidiDeviceInId = deviceId;
                    mMidiChannelButtons = deviceInfo->mChannelButtons;
                    mNodeName = deviceInfo->GetName();
                    mNodeName += " : Pads";
                    mNodeName += std::to_string(mButtonGroup);
                }
            }
        }

        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mMidiDeviceInId = -1;
        int32_t mMidiChannelButtons = -1;

        int32_t mCallbackId = 0;
        int32_t mButtonGroup;
        std::string mNodeName;
        bool mMidiShiftState = false;
        std::vector<int8_t> mButtonStates;
        int32_t mInitCounter = 0;
    };
}

