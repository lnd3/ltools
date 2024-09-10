#pragma once

#include "logging/Log.h"

#include "MidiDefs.h"

#include <memory>

namespace l::hid::midi {

	struct MidiData {
		uint32_t msg; // input message
		uint32_t deviceIn; // device id
		uint32_t deviceOut; // corresponding out device 
		uint32_t timestamp; // Since opening the midi device
		uint32_t status; // noteon=145, noteoff=129, knob=176, padon=144, padoff=128, sustain=177,
		uint32_t channel;
		uint32_t data1; // Key unique
		uint32_t data2; // Key attack (0-127), pad attack (127), knob position (0-127)
		uint32_t unused;
	};


	struct DeviceInfo {
		std::string mName;
		int32_t mInDevice = -1;
		int32_t mOutDevice = -1;
		int32_t mChannelKeys = -1;
		int32_t mChannelButtons = -1;
		int32_t mChannelKnobs = -1;

		std::string_view GetName() {
			return mName;
		}
		bool HasMidiIn() {
			return mInDevice >= 0;
		}
		bool HasMidiOut() {
			return mInDevice >= 0;
		}
		uint32_t GetMidiIn() {
			return static_cast<uint32_t>(mInDevice);
		}
		uint32_t GetMidiOut() {
			return static_cast<uint32_t>(mOutDevice);
		}
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

	struct MidiHandle {
		int32_t mCallbackId = 0;
		uint32_t mMidiDeviceInId = 0;
		uint32_t mMidiDeviceOutId = 0;
	};

	namespace details {
		extern std::mutex midiCallbackMutex;
		extern int32_t midiGuid;
		extern std::unordered_map<int32_t, l::hid::midi::CallbackFunction> midiCallback;

		void HandleMidiData(uint32_t msg, uint32_t deviceIn, uint32_t deviceOut, uint32_t param1, uint32_t param2);
	}

	class MidiManager {
	public:
		MidiManager() = default;
		virtual ~MidiManager() {
			ClearCallbacks();
		};

		virtual void ClearCallbacks();
		virtual void RescanMidiDevices();
		virtual int32_t RegisterCallback(CallbackFunction cb);
		virtual void UnregisterCallback(int32_t id);
		virtual uint32_t GetNumDevices();
		virtual std::string_view GetDeviceName(uint32_t);
		virtual DeviceInfo* GetDeviceInfo(uint32_t deviceId);
		virtual void SendToDevice(uint32_t deviceIndex, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2);
	};

	std::unique_ptr<MidiManager> CreateMidiManager();

}


