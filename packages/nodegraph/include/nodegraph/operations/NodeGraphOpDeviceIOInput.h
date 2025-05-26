#pragma once
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/LoggingAll.h"

#include "hid/KeyboardPiano.h"
#include "hid/KeyState.h"
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
    class GraphInputKeyboardPiano : public NodeGraphOp, public l::audio::INoteProcessor {
    public:
        GraphInputKeyboardPiano(NodeGraphBase* node, l::hid::KeyState* keyState) :
            NodeGraphOp(node, "Keyboard")
        {
            AddOutput("Freq", 0.0f);
            AddOutput("Note On", l::audio::gNoNote_f, 8);
            AddOutput("Note Off", l::audio::gNoNote_f, 8);

            AddConstant("Freq", 0.0f);
            AddConstant("Note On", l::audio::gNoNote_f, 8);
            AddConstant("Note Off", l::audio::gNoNote_f, 8);

            mChannel.resize(1);
            mKeyboard.SetKeyState(keyState);
            mKeyboard.SetNoteProcessor(this);
        }

        virtual ~GraphInputKeyboardPiano() {
            mKeyboard.SetKeyState(nullptr);
            mKeyboard.SetNoteProcessor(nullptr);
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tickCount, float elapsed) override;
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
    class GraphInputMidiKeyboard : public NodeGraphOp, public l::audio::INoteProcessor {
    public:
        const static int32_t gPolyphony = 12;

        GraphInputMidiKeyboard(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, ""),
            mMidiManager(midiManager)
        {
            AddOutput("Freq", 0.0f);
            AddOutput("Velocity", 0.0f);
            AddOutput("Note On", l::audio::gNoNote_f, gPolyphony);
            AddOutput("Note Off", l::audio::gNoNote_f, gPolyphony);
            AddOutput("Device Id", 0.0f);

            AddConstant("Freq", 0.0f);
            AddConstant("Velocity", 0.0f);
            AddConstant("Note On", l::audio::gNoNote_f, gPolyphony);
            AddConstant("Note Off", l::audio::gNoNote_f, gPolyphony);
            AddConstant("Device Id", 0.0f, 1, 0.0f, 10.0f);

            mChannel.resize(1);

            SetDevice(0);

            if (mMidiManager) {
                mCallbackId = mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                    if (mMidiDeviceInId >= 0 && data.deviceIn == static_cast<uint32_t>(mMidiDeviceInId)) {
                        MidiEvent(data);
                    }
                    });
            }
        }

        virtual ~GraphInputMidiKeyboard() {
            if (mMidiManager != nullptr) {
                mMidiManager->UnregisterCallback(mCallbackId);
            }
        }

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tick, float deltaTime) override;

        virtual void MidiEvent(const l::hid::midi::MidiData& data);
        virtual void NoteOn(int32_t note, int32_t velocity = 127) override;
        virtual void NoteOff() override;
        virtual void NoteOff(int32_t note) override;
        virtual void NoteSustain(bool on) override;
    protected:
        int8_t ResetNoteChannel(int32_t note);
        int8_t GetNextNoteChannel(int32_t note);

        void SetDevice(int32_t deviceId) {
            if (mMidiManager != nullptr && deviceId >= 0 && mMidiDeviceInId != deviceId) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(static_cast<uint32_t>(deviceId));
                if (deviceInfo) {
                    mMidiDeviceInId = deviceId;
                    mMidiChannelKeys = deviceInfo->mChannelKeys;
                    mName = deviceInfo->GetName();
                    mName += " : Midi";
                }
            }
        }

        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mCallbackId = 0;

        int32_t mLastNote = 0;
        int8_t mNoteCounter = 0;
        bool mSustain = false;
        std::vector<std::pair<int32_t, int32_t>> mChannel;
        int32_t mMidiDeviceInId = -1;
        int32_t mMidiChannelKeys = -1;
        std::vector<int32_t> mSustainedNotes;
    };

    /*********************************************************************/
    class GraphInputMidiKnobs : public NodeGraphOp {
    public:
        GraphInputMidiKnobs(NodeGraphBase* node, l::hid::midi::MidiManager* midiManager) :
            NodeGraphOp(node, ""),
            mMidiManager(midiManager)
        {
            for (int32_t i = 0; i < 8; i++) {
                AddOutput("Knob " + std::to_string(i), 0.0f);
            }
            for (int32_t i = 0; i < 8; i++) {
                AddConstant("Knob " + std::to_string(i), 0.0f);
            }
            AddOutput("Device Id", 0.0f);
            AddConstant("Device Id", 0.0f, 1, 0.0f, 10.0f);

            SetDevice(0);

            if (mMidiManager != nullptr) {
                mCallbackId = mMidiManager->RegisterCallback([&](l::hid::midi::MidiData data) {
                    if (mMidiDeviceInId >= 0 && data.deviceIn == static_cast<uint32_t>(mMidiDeviceInId)) {
                        MidiEvent(data);
                    }
                    });
            }
        }

        virtual ~GraphInputMidiKnobs() {
            if (mMidiManager != nullptr) {
                mMidiManager->UnregisterCallback(mCallbackId);
            }
        };

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tick, float deltaTime) override;
        virtual void MidiEvent(const l::hid::midi::MidiData& data);
    protected:
        void SetDevice(int32_t deviceId) {
            if (mMidiManager != nullptr && deviceId >= 0 && mMidiDeviceInId != deviceId) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(static_cast<uint32_t>(deviceId));
                if (deviceInfo) {
                    mMidiDeviceInId = deviceId;
                    mMidiChannelKnobs = deviceInfo->mChannelKnobs;
                    mName = deviceInfo->GetName();
                    mName += " : Knobs";
                }
            }
        }

        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mCallbackId = 0;

        int32_t mMidiDeviceInId = -1;
        int32_t mMidiChannelKnobs = -1;
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
            NodeGraphOp(node, ""),
            mMidiManager(midiManager),
            mButtonGroup(buttonGroup)
        {
            for (int32_t i = 0; i < 8; i++) {
                AddOutput("Pad " + std::to_string(i), 0.0f);
            }
            for (int32_t i = 0; i < 8; i++) {
                AddConstant("Pad " + std::to_string(i), 0.0f);
            }
            AddOutput("Device Id", 0.0f);
            AddConstant("Device Id", 0.0f, 1, 0.0f, 10.0f);

            mButtonStates.resize(8);

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

            if (mMidiManager != nullptr) {
                mMidiManager->UnregisterCallback(mCallbackId);
            }
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t tick, float deltaTime) override;

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
            if (mMidiManager != nullptr && deviceId >= 0 && mMidiDeviceInId != deviceId) {
                auto deviceInfo = mMidiManager->GetDeviceInfo(static_cast<uint32_t>(deviceId));
                if (deviceInfo) {
                    mMidiDeviceInId = deviceId;
                    mMidiChannelButtons = deviceInfo->mChannelButtons;
                    mName = deviceInfo->GetName();
                    mName += " : Pads";
                    mName += std::to_string(mButtonGroup);
                }
            }
        }

        l::hid::midi::MidiManager* mMidiManager = nullptr;
        int32_t mMidiDeviceInId = -1;
        int32_t mMidiChannelButtons = -1;

        int32_t mCallbackId = 0;
        int32_t mButtonGroup;
        bool mMidiShiftState = false;
        std::vector<int8_t> mButtonStates;
        int32_t mInitCounter = 0;
    };

    /*********************************************************************/
    class GraphInputMic : public NodeGraphOp {
    public:
        GraphInputMic(NodeGraphBase* node, l::audio::AudioStream* stream = nullptr) :
            NodeGraphOp(node, "Mic"),
            mAudioStream(stream),
            mCurrentStereoPosition(0)
        {
            AddInput("Volume", 0.5f, 1, 0.0f, 1.0f);

            AddOutput("Left");
            AddOutput("Right");

            mEnvelope = 0.0f;
            mFilterEnvelope.SetConvergenceFactor();
        }
        virtual ~GraphInputMic() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        l::audio::AudioStream* mAudioStream;
        float mSamplesUntilUpdate = 0.0f;

        int32_t mCurrentStereoPosition;
        float mEnvelope = 0.0f;
        float mAttack = 1.0f;
        float mRelease = 1.0f;
        l::audio::FilterRWA<float> mFilterEnvelope;
    };


}

