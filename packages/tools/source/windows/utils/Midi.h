#pragma once

#include "logging/Log.h"

#include <windows.h> 
#include <mmsystem.h>

namespace l {
namespace x {

namespace midi {

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
	using CallbackFunction = std::function<void(MidiData)>;
	namespace {
		static std::mutex midiCallbackMutex;
		static std::vector<CallbackFunction*> midiCallback;
	}

	namespace details {
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

				for (auto cb : midiCallback) {
					if (cb) {
						(*cb)(data);
					}
				}
			}
		}

		void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
			HandleMidiData(static_cast<uint32_t>(wMsg), static_cast<uint32_t>(dwInstance), static_cast<uint32_t>(dwParam1), static_cast<uint32_t>(dwParam2));
		}

		void CALLBACK MidiOutProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

			switch (wMsg) {
			case MOM_OPEN:
				LOG(LogInfo) << "MOM_OPEN";
				break;
			case MOM_CLOSE:
				LOG(LogInfo) << "MOM_CLOSE";
				break;
			case MOM_DONE:
				LOG(LogInfo) << "MOM_DONE";
				break;
			default:
				LOG(LogInfo) << "Something else..";
				break;
			}

		}
	}

	class Midi {
	public:
		Midi(CallbackFunction f) : mHeader{}, mMidiBuffer{}, mCallback(std::move(f)) {
			{
				std::lock_guard<std::mutex> lock(midiCallbackMutex);
				midiCallback.push_back(&mCallback);
			}

			UINT nMidiDeviceNum = midiInGetNumDevs();
			if (nMidiDeviceNum == 0) {
				return;
			}
			UINT nMidiOutDeviceNum = midiOutGetNumDevs();

			for (size_t deviceId = 0; deviceId < nMidiDeviceNum; deviceId++) {
				MMRESULT rv;
				HMIDIIN hMidiDevice = NULL;

				rv = midiInOpen(&hMidiDevice, static_cast<UINT>(deviceId), reinterpret_cast<DWORD_PTR>(&details::MidiInProc), 0, CALLBACK_FUNCTION);
				if (rv == MMSYSERR_ALLOCATED) {
					return;
				}
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to open midi in device " << deviceId;
					continue;
				}

				rv = midiInStart(hMidiDevice);
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to start midi in device" << deviceId;
					continue;
				}

				MIDIINCAPS capsIn;
				rv = midiInGetDevCaps(deviceId, &capsIn, sizeof(MIDIINCAPS));
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to get midi in caps on device " << deviceId;
				}

				HMIDIOUT hMidiDeviceOut = nullptr;
				deviceId++;
				rv = midiOutOpen(&hMidiDeviceOut, static_cast<UINT>(deviceId), reinterpret_cast<DWORD_PTR>(&details::MidiOutProc), 0, CALLBACK_FUNCTION);
				if (rv == MMSYSERR_ALLOCATED) {
					return;
				}
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to open midi out device " << deviceId;
				}

				MIDIOUTCAPS capsOut;
				rv = midiOutGetDevCaps(deviceId, &capsOut, sizeof(MIDIOUTCAPS));
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to get midi out caps on device " << deviceId;
				}
				deviceId--;

				devices.push_back(std::make_pair(hMidiDevice, hMidiDeviceOut));
				caps.push_back(std::make_pair(capsIn, capsOut));
			}
		}
		~Midi() {
			int deviceId = 0;
			for (auto device : devices) {
				MMRESULT rv;
				rv = midiOutClose(device.second);
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to close midi out device" << deviceId;
				}
				rv = midiInStop(device.first);
				if (rv == MMSYSERR_NOERROR) {
					rv = midiInClose(device.first);
				}
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to close midi in device" << deviceId;
				}
				deviceId++;
			}
			{
				std::lock_guard<std::mutex> lock(midiCallbackMutex);
				auto it = std::find(midiCallback.begin(), midiCallback.end(), &mCallback);
				if (it != midiCallback.end()) {
					midiCallback.erase(it);
				}
			}
		}

		void clearBuffer() {
			mHeader.dwBufferLength = 0;
			mHeader.dwBytesRecorded = 0;
		}

		void pushBuffer(unsigned char byte) {
			mMidiBuffer[mHeader.dwBufferLength] = static_cast<char>(byte);
			mHeader.dwBufferLength++;
			mHeader.dwBytesRecorded++;
		}

		void sendSysex2(uint32_t deviceId, uint32_t i, uint32_t data) {
			if (deviceId >= devices.size()) {
				return;
			}
			static uint32_t counter = 0;

			mHeader = { 0 };
			mHeader.lpData = mMidiBuffer;

			clearBuffer();

			switch (i) {
			case 0:
				mMidiBuffer[0] = 0x7e; // 
				mMidiBuffer[1] = 0x00; // AKAI
				mMidiBuffer[2] = 0x06; // General information
				mMidiBuffer[3] = 0x01; // Identity request
				mHeader.dwBufferLength = 4;
				mHeader.dwBytesRecorded = 4;
				break;
			case 1:
				pushBuffer(0xf0);
				pushBuffer(0x47);
				pushBuffer(0x00);
				pushBuffer(0xff); // 27
				pushBuffer(0xf7);
				break;
			case 2:
				pushBuffer(0xf0);
				pushBuffer(0x47);
				pushBuffer(0xff);
				pushBuffer(0xff);
				pushBuffer(0x60);
				pushBuffer(0x00);
				pushBuffer(0x04);
				pushBuffer(0x40);
				pushBuffer(0x01);
				pushBuffer(0x00);
				pushBuffer(0x00);
				pushBuffer(0xf7);
				break;
			case 3:
				mMidiBuffer[0] = 0x7f; // Real Time (7FH)
				mMidiBuffer[1] = 0x47; // AKAI
				mMidiBuffer[2] = 0x02; // MIDI Show Control
				mMidiBuffer[3] = data;
				mHeader.dwBufferLength = 4;
				mHeader.dwBytesRecorded = 4;
				break;
			case 4:
				mMidiBuffer[0] = 0x7f; // Real Time (7FH)
				mMidiBuffer[1] = 0x47; // AKAI
				mMidiBuffer[2] = 0x06; // MIDI Machine Control Commands
				mMidiBuffer[3] = data;
				mHeader.dwBufferLength = 4;
				mHeader.dwBytesRecorded = 4;
				break;
			default:

				break;
			}

			HMIDIIN device = devices[deviceId].first;
			HMIDIOUT deviceOut = devices[deviceId].second;

			MMRESULT rv = midiOutPrepareHeader(deviceOut, &mHeader, sizeof(MIDIHDR));
			if (rv != MMSYSERR_NOERROR) {
				LOG(LogError) << "Failed to prepare midi out buffer " << deviceId << ", error " << rv;
			}

			rv = midiOutLongMsg(deviceOut, &mHeader, sizeof(MIDIHDR));
			if (rv != MMSYSERR_NOERROR) {
				LOG(LogError) << "Failed to send buffer to midi out device " << deviceId << ", error " << rv;
			}

			rv = midiOutUnprepareHeader(deviceOut, &mHeader, sizeof(MIDIHDR));
			if (rv != MMSYSERR_NOERROR) {
				LOG(LogError) << "Failed to unprepare midi out buffer " << deviceId << ", error " << rv;
			}
		}

		void send(uint32_t deviceId, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2) {
			if (deviceId >= devices.size()) {
				return;
			}
			HMIDIOUT device = devices[deviceId].second;

			DWORD param1 = ((data2 & 0xff) << 16) | ((data1 & 0xff) << 8) | ((status & 0xf0) | (channel & 0xf));

			MMRESULT rv = midiOutShortMsg(device, param1);
			if (rv != MMSYSERR_NOERROR) {
				LOG(LogError) << "Failed to send to midi out device " << deviceId << ", error " << rv << ", data " << param1;
			}
		}

	private:
		MIDIHDR mHeader{};
		char mMidiBuffer[16];

		CallbackFunction mCallback;
		std::vector<std::pair<HMIDIIN, HMIDIOUT>> devices;
		std::vector< std::pair<MIDIINCAPS, MIDIOUTCAPS>> caps;
	};

}

}
}

