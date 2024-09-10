#include "logging/Log.h"

#include "MidiLinux.h"

namespace l::hid::midi {

	std::unique_ptr<MidiManager> CreateMidiManager() {
		return nullptr;
	}

	void MidiManagerLinux::RescanMidiDevices() {}

	uint32_t MidiManagerLinux::GetNumDevices() {
		return 0;
	}

	void MidiManagerLinux::SendToDevice(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
	}
}

