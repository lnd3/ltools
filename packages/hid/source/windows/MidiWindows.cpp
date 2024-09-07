#include "logging/Log.h"

#include "MidiWindows.h"

namespace l::hid::midi {

	std::unique_ptr<MidiManager> CreateMidiManager() {
		return std::make_unique<MidiManagerWindows>();
	}

	namespace details {
		void CALLBACK MidiInProc(HMIDIIN, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
			HandleMidiData(static_cast<uint32_t>(wMsg), static_cast<uint32_t>(dwInstance), static_cast<uint32_t>(dwParam1), static_cast<uint32_t>(dwParam2));
		}

		void CALLBACK MidiOutProc(HMIDIIN, UINT wMsg, DWORD, DWORD, DWORD) {
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



	Midi::Midi() : mHeader{}, mMidiBuffer{} {
		UINT nMidiDeviceNum = midiInGetNumDevs();
		if (nMidiDeviceNum == 0) {
			return;
		}

		UINT nMidiOutDeviceNum = midiOutGetNumDevs();

		LOG(LogInfo) << "Number of midi in devices: " << nMidiDeviceNum;
		LOG(LogInfo) << "Number of midi out devices: " << nMidiOutDeviceNum;

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

	Midi::~Midi() {
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
			std::lock_guard<std::mutex> lock(details::midiCallbackMutex);
			details::midiCallback.clear();
		}
	}

	void Midi::registerMidiCallback(CallbackFunction f) {
		std::lock_guard<std::mutex> lock(details::midiCallbackMutex);
		details::midiCallback.push_back(f);
	}

	uint32_t Midi::getNumDevices() {
		return static_cast<uint32_t>(devices.size());
	}

	void Midi::clearBuffer() {
		mHeader.dwBufferLength = 0;
		mHeader.dwBytesRecorded = 0;
	}

	void Midi::pushBuffer(unsigned char byte) {
		mMidiBuffer[mHeader.dwBufferLength] = static_cast<char>(byte);
		mHeader.dwBufferLength++;
		mHeader.dwBytesRecorded++;
	}

	void Midi::sendSysex2(uint32_t deviceId, uint32_t i, uint32_t data) {
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
			mMidiBuffer[3] = static_cast<char>(data);
			mHeader.dwBufferLength = 4;
			mHeader.dwBytesRecorded = 4;
			break;
		case 4:
			mMidiBuffer[0] = 0x7f; // Real Time (7FH)
			mMidiBuffer[1] = 0x47; // AKAI
			mMidiBuffer[2] = 0x06; // MIDI Machine Control Commands
			mMidiBuffer[3] = static_cast<char>(data);
			mHeader.dwBufferLength = 4;
			mHeader.dwBytesRecorded = 4;
			break;
		default:

			break;
		}

		//HMIDIIN deviceIn = devices[deviceId].first;
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

	void Midi::send(uint32_t deviceId, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2) {
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

	void MidiManagerWindows::RegisterCallback(CallbackFunction cb) {
		mMidiDevice.registerMidiCallback(cb);
	}

	uint32_t MidiManagerWindows::GetNumDevices() {
		return mMidiDevice.getNumDevices();
	}

	void MidiManagerWindows::SendToDevice(uint32_t deviceIndex, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2) {
		mMidiDevice.send(deviceIndex, status, channel, data1, data2);
	}
}

