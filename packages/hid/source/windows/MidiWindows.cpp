#include "logging/Log.h"

#include "MidiWindows.h"

namespace l::hid::midi {

	std::unique_ptr<MidiManager> CreateMidiManager() {
		return std::make_unique<MidiManagerWindows>();
	}

	namespace details {
		void CALLBACK MidiInProc(HMIDIIN deviceIn, UINT wMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2) {
			auto midiPtr = reinterpret_cast<Midi*>(dwInstance);
			auto deviceInfo = midiPtr->getDeviceInfo(deviceIn);
			if (deviceInfo) {
				HandleMidiData(static_cast<uint32_t>(wMsg), deviceInfo->GetMidiIn(), deviceInfo->GetMidiOut(), static_cast<uint32_t>(dwParam1), static_cast<uint32_t>(dwParam2));
			}
		}

		void CALLBACK MidiOutProc(HMIDIOUT, UINT wMsg, DWORD_PTR, DWORD, DWORD) {
			//auto midiPtr = reinterpret_cast<Midi*>(dwInstance);
			//auto id = midiPtr->getDeviceOutId(deviceOut);
			switch (wMsg) {
			case MOM_OPEN:
				//LOG(LogInfo) << "MOM_OPEN";
				break;
			case MOM_CLOSE:
				//LOG(LogInfo) << "MOM_CLOSE";
				break;
			case MOM_DONE:
				//LOG(LogInfo) << "MOM_DONE";
				break;
			default:
				//LOG(LogInfo) << "Something else..";
				break;
			}

		}
	}

	Midi::Midi() : mHeader{}, mMidiBuffer{} {
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

	void Midi::initDevices() {
		mDeviceInfo.clear();
		devices.clear();
		caps.clear();

		UINT nMidiInDeviceInNum = midiInGetNumDevs();
		if (nMidiInDeviceInNum == 0) {
			return;
		}

		UINT nMidiOutDeviceNum = midiOutGetNumDevs();

		LOG(LogInfo) << "Number of midi in devices: " << nMidiInDeviceInNum;
		LOG(LogInfo) << "Number of midi out devices: " << nMidiOutDeviceNum;

		for (uint32_t deviceId = 0; deviceId < nMidiInDeviceInNum || deviceId < nMidiOutDeviceNum; deviceId++) {
			MMRESULT rv;
			HMIDIIN hMidiDeviceIn = nullptr;
			HMIDIOUT hMidiDeviceOut = nullptr;
			MIDIINCAPS capsIn = { 0 };
			MIDIOUTCAPS capsOut = { 0 };

			if (deviceId < nMidiInDeviceInNum) {
				rv = midiInGetDevCaps(deviceId, &capsIn, sizeof(MIDIINCAPS));
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to get midi in caps on device " << deviceId;
				}
				LOG(LogInfo) << "Midi in device id " << deviceId << " : " << capsIn.szPname << ", support : " << capsIn.dwSupport << ", pid : " << capsIn.wPid;

				rv = midiInOpen(&hMidiDeviceIn, static_cast<UINT>(deviceId), reinterpret_cast<DWORD_PTR>(&details::MidiInProc), (DWORD_PTR)(this), CALLBACK_FUNCTION | MIDI_IO_STATUS);
				if (rv == MMSYSERR_ALLOCATED) {
					return;
				}
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to open midi in device " << deviceId;
					continue;
				}

				rv = midiInStart(hMidiDeviceIn);
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to start midi in device" << deviceId;
					continue;
				}
			}

			if (deviceId < nMidiOutDeviceNum) {
				rv = midiOutGetDevCaps(deviceId, &capsOut, sizeof(MIDIOUTCAPS));
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to get midi out caps on device " << deviceId;
				}
				LOG(LogInfo) << "Midi out device id " << deviceId << " : " << capsOut.szPname << ", support : " << capsOut.dwSupport << ", pid : " << capsOut.wPid;

				rv = midiOutOpen(&hMidiDeviceOut, static_cast<UINT>(deviceId), reinterpret_cast<DWORD_PTR>(&details::MidiOutProc), (DWORD_PTR)(this), CALLBACK_FUNCTION);
				if (rv == MMSYSERR_ALLOCATED) {
					return;
				}
				if (rv != MMSYSERR_NOERROR) {
					LOG(LogError) << "Failed to open midi out device " << deviceId;
				}
			}

			devices.push_back(std::make_pair(hMidiDeviceIn, hMidiDeviceOut));
			caps.push_back(std::make_pair(capsIn, capsOut));

			if (deviceId + 1 >= mDeviceInfo.size()) {
				mDeviceInfo.resize(deviceId + 1);
			}
			if (hMidiDeviceIn != nullptr) {
				auto& deviceInfo = mDeviceInfo.at(deviceId);
				deviceInfo.mInDevice = static_cast<int32_t>(deviceId);
				if (capsIn.szPname) {
					deviceInfo.mName = capsIn.szPname;

					if (deviceInfo.mName == "APC Key 25") {
						deviceInfo.mChannelKeys = 1;
						deviceInfo.mChannelButtons = 0;
						deviceInfo.mChannelKnobs = 0;
					}
					else if (deviceInfo.mName == "Keystation Mini 32") {
						deviceInfo.mChannelKeys = 0;
						deviceInfo.mChannelButtons = 0;
						deviceInfo.mChannelKnobs = 0;
					}
					else {
						deviceInfo.mChannelKeys = 0;
						deviceInfo.mChannelButtons = 0;
						deviceInfo.mChannelKnobs = 0;
					}
				}
			}
			if (hMidiDeviceOut != nullptr) {
				for (int32_t i = 0; i < mDeviceInfo.size(); i++) {
					auto& deviceInfo = mDeviceInfo.at(i);
					if (std::string(capsOut.szPname) == deviceInfo.GetName()) {
						deviceInfo.mOutDevice = static_cast<int32_t>(deviceId);
					}
				}
			}
		}
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

	DeviceInfo* Midi::getDeviceInfo(HMIDIIN midiIn) {
		for (int32_t i = 0; i < devices.size() && mDeviceInfo.size(); i++) {
			if (devices.at(i).first == midiIn) {
				return &mDeviceInfo.at(i);
			}
		}
		return nullptr;
	}

	DeviceInfo* Midi::getDeviceInfo(uint32_t deviceId) {
		for (int32_t i = 0; i < mDeviceInfo.size(); i++) {
			auto& deviceInfo = mDeviceInfo.at(i);
			if (deviceInfo.GetMidiIn() == deviceId) {
				return &deviceInfo;
			}
		}
		return 0;
	}

	MidiManagerWindows::MidiManagerWindows() {
		RescanMidiDevices();
	}

	void MidiManagerWindows::RescanMidiDevices() {
		ClearCallbacks();
		mMidiDevice.initDevices();

		RegisterCallback([](const MidiData&) {
			//LOG(LogInfo) << "midi cb: device in:" << data.deviceIn << " device out:" << data.deviceOut << " stat:" << data.status << " ch:" << data.channel << " d1:" << data.data1 << " d2:" << data.data2;
			});
	}

	uint32_t MidiManagerWindows::GetNumDevices() {
		return mMidiDevice.getNumDevices();
	}

	std::string_view MidiManagerWindows::GetDeviceName(uint32_t deviceId) {
		auto deviceInfo = mMidiDevice.getDeviceInfo(deviceId);
		if (deviceInfo != nullptr) {
			return deviceInfo->GetName();
		}
		return "";
	}

	DeviceInfo* MidiManagerWindows::GetDeviceInfo(uint32_t deviceId) {
		return mMidiDevice.getDeviceInfo(deviceId);
	}

	void MidiManagerWindows::SendToDevice(uint32_t deviceIndex, uint32_t status, uint32_t channel, uint32_t data1, uint32_t data2) {
		mMidiDevice.send(deviceIndex, status, channel, data1, data2);
	}
}

