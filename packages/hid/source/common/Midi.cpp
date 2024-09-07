#include "logging/Log.h"

#include "hid/Midi.h"

#include <unordered_map>

namespace l::hid::midi {

	namespace details {
		std::mutex midiCallbackMutex;
		int32_t midiGuid = 0;
		std::unordered_map<int32_t, l::hid::midi::CallbackFunction> midiCallback;

		void HandleMidiData(uint32_t msg, uint32_t instance, uint32_t param1, uint32_t param2) {
			switch (msg) {
			case MIM_OPEN:
				LOG(LogInfo) << "MIM_OPEN";
				return;
			case MIM_CLOSE:
				LOG(LogInfo) << "MIM_CLOSE";
				return;
			case MIM_LONGDATA:
				LOG(LogInfo) << "Long data: " << msg << " " << instance << " " << param1 << " " << param2;
				return;
			case MIM_ERROR:
				LOG(LogInfo) << "Error: " << msg << " " << instance << " " << param1 << " " << param2;
				return;
			case MIM_LONGERROR:
				LOG(LogInfo) << "Long error: " << msg << " " << instance << " " << param1 << " " << param2;
				return;
			case MIM_MOREDATA:
				LOG(LogInfo) << "More data: " << msg << " " << instance << " " << param1 << " " << param2;
				return;
			case MIM_DATA:
				//LOG(LogInfo) << "Data: " << msg << " " << instance << " " << param1 << " " << param2;
				break;
			default:
				LOG(LogInfo) << "default";
				break;
			}

			std::lock_guard<std::mutex> lock(midiCallbackMutex);
			if (!midiCallback.empty()) {
				MidiData data;
				data.msg = msg;
				data.device = instance;
				data.timestamp = param2;
				data.status = (param1 >> 4) & 0xf;
				data.channel = (param1) & 0xf;
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

	void MidiManager::SendToDevice(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
	}
}
