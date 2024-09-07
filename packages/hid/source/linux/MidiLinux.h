#pragma once

#include "logging/Log.h"

#include "hid/Midi.h"

namespace l::hid::midi {

	class MidiManagerLinux : public MidiManager {
	public:
		MidiManagerLinux() = default;

		virtual void RegisterCallback(CallbackFunction cb) override;
		virtual uint32_t GetNumDevices() override;
		virtual void SendToDevice(uint32_t deviceIndex, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2) override;
	protected:
	};

}


