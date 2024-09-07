#pragma once

#include "logging/Log.h"

#include "hid/Midi.h"

#include <windows.h> 
#include <mmsystem.h>

namespace l::hid::midi {


	namespace details {
		void CALLBACK MidiInProc(HMIDIIN, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
		void CALLBACK MidiOutProc(HMIDIIN, UINT wMsg, DWORD, DWORD, DWORD);
	}

	class Midi {
	public:
		Midi();
		~Midi();
		
		void registerMidiCallback(CallbackFunction f);
		uint32_t getNumDevices();
		void clearBuffer();
		void pushBuffer(unsigned char byte);
		void sendSysex2(uint32_t deviceId, uint32_t i, uint32_t data);
		void send(uint32_t deviceId, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2);

	private:
		MIDIHDR mHeader{};
		char mMidiBuffer[16];

		CallbackFunction mCallback;
		std::vector<std::pair<HMIDIIN, HMIDIOUT>> devices;
		std::vector< std::pair<MIDIINCAPS, MIDIOUTCAPS>> caps;
	};


	class MidiManagerWindows : public MidiManager {
	public:
		MidiManagerWindows() = default;

		virtual void RegisterCallback(CallbackFunction cb) override;
		virtual uint32_t GetNumDevices() override;
		virtual void SendToDevice(uint32_t deviceIndex, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2) override;
	protected:
		Midi mMidiDevice;
	};

}


