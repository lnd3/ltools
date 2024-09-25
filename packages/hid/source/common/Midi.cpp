#include "logging/Log.h"

#include "hid/Midi.h"

#include <unordered_map>

namespace l::hid::midi {

	namespace details {
		std::mutex midiCallbackMutex;
		int32_t midiGuid = 0;
		std::unordered_map<int32_t, l::hid::midi::CallbackFunction> midiCallback;

		void HandleMidiData(uint32_t msg, uint32_t deviceId, uint32_t deviceOutId, uint32_t param1, uint32_t param2) {
			switch (msg) {
			case MIM_OPEN:
				//LOG(LogInfo) << "MIM_OPEN";
				return;
			case MIM_CLOSE:
				//LOG(LogInfo) << "MIM_CLOSE";
				return;
			case MIM_LONGDATA:
				LOG(LogInfo) << "Long data: " << msg << " " << deviceId << " " << param1 << " " << param2;
				return;
			case MIM_ERROR:
				LOG(LogInfo) << "Error: " << msg << " " << deviceId << " " << param1 << " " << param2;
				return;
			case MIM_LONGERROR:
				LOG(LogInfo) << "Long error: " << msg << " " << deviceId << " " << param1 << " " << param2;
				return;
			case MIM_MOREDATA:
				LOG(LogInfo) << "More data: " << msg << " " << deviceId << " " << param1 << " " << param2;
				return;
			case MIM_DATA:
				//LOG(LogInfo) << "Data: " << msg << " " << deviceId << " " << param1 << " " << param2;
				break;
			default:
				LOG(LogInfo) << "default";
				break;
			}

			std::lock_guard<std::mutex> lock(midiCallbackMutex);
			if (!midiCallback.empty()) {
				MidiData data;
				data.msg = msg;
				data.deviceIn = deviceId;
				data.deviceOut = deviceOutId;
				data.timestamp = param2;
				data.channel = (param1) & 0xf;
				data.status = (param1 >> 4) & 0xf;
				data.data1 = (param1 >> 8) & 0xff;
				data.data2 = (param1 >> 16) & 0xff;
				data.unused = 0;

				for (auto& cb : midiCallback) {
					if (cb.second) {
						cb.second(data);
					}
				}
			}
		}
	}

	void MidiManager::ClearCallbacks() {
		details::midiCallback.clear();
	}

	void MidiManager::RescanMidiDevices() {}

	int32_t MidiManager::RegisterCallback(CallbackFunction f) {
		std::lock_guard<std::mutex> lock(details::midiCallbackMutex);
		int32_t id = details::midiGuid++;

		
		details::midiCallback.emplace(id, f);
		return id;
	}

	void MidiManager::UnregisterCallback(int32_t id) {
		details::midiCallback.erase(id);
	}

	uint32_t MidiManager::GetNumDevices() {
		return 0;
	}

	std::string_view MidiManager::GetDeviceName(uint32_t) {
		return "";
	}

	DeviceInfo* MidiManager::GetDeviceInfo(uint32_t) {
		return nullptr;
	}

	void MidiManager::SendToDevice(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
	}
}

