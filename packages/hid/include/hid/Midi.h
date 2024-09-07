#pragma once

#include "logging/Log.h"

#include "MidiDefs.h"

#include <memory>

namespace l::hid::midi {

	struct MidiData {
		uint32_t msg; // input message
		uint32_t device; // device id
		uint32_t timestamp; // Since opening the midi device
		uint32_t status; // noteon=145, noteoff=129, knob=176, padon=144, padoff=128, sustain=177,
		uint32_t channel;
		uint32_t data1; // Key unique
		uint32_t data2; // Key attack (0-127), pad attack (127), knob position (0-127)
		uint32_t unused;
	};

	/*
		uint32_t msg; // message
		uint32_t instance; // device id
		uint32_t type = (param1) & 0xff; // noteon=145, noteoff=129, knob=176, padon=144, padoff=128, sustain=177,
		uint32_t key = (param1 >> 8) & 0xff; // key unique
		uint32_t atc = (param1 >> 16) & 0xff; // key attack (0-127), pad attack (127), knob position (0-127)
		uint32_t arg2 = (param1 >> 24) & 0xff; // unused?
		uint32_t time = param2;
	*/
	using CallbackFunction = std::function<void(const MidiData&)>;

	namespace details {
		extern std::mutex midiCallbackMutex;
		extern std::vector<l::hid::midi::CallbackFunction> midiCallback;

		void HandleMidiData(uint32_t msg, uint32_t instance, uint32_t param1, uint32_t param2);
	}

	class MidiManager {
	public:
		MidiManager() {
			RegisterCallback([](const MidiData& data) {
				LOG(LogInfo) << "listener 1: dev" << data.device << " stat " << data.status << " ch " << data.channel << " d1 " << data.data1 << " d2 " << data.data2;
				});
		}
		virtual ~MidiManager() = default;

		virtual void RegisterCallback(CallbackFunction cb);
		virtual uint32_t GetNumDevices();
		virtual void SendToDevice(uint32_t deviceIndex, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2);
	};

	std::unique_ptr<MidiManager> CreateMidiManager();

}


