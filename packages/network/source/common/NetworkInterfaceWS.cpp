#include "network/NetworkInterfaceWS.h"

#include "logging/String.h"

#include <memory>

namespace l::network {

	std::shared_ptr<NetworkInterfaceWS> CreateNetworkInterfaceWS(std::weak_ptr<l::network::NetworkManager> networkManager) {
		return std::make_shared<NetworkInterfaceWS>(networkManager);
	}

	void NetworkInterfaceWS::CreateInterface(std::string_view interfaceName, 
		std::string_view protocol, 
		std::string_view host, 
		uint32_t port, 
		int32_t networkStatusInterval) {
		if (!mInterfaces.contains(interfaceName.data())) {
			mInterfaces.emplace(std::string(interfaceName), HostInfo(protocol, host, port, networkStatusInterval) );
		}
	}

	void NetworkInterfaceWS::Shutdown() {
		auto networkManager = mNetworkManager.lock();
		if (networkManager) {
			networkManager->WSClose();
		}
	}

	void NetworkInterfaceWS::Disconnect(std::string_view interfaceName) {
		auto networkManager = mNetworkManager.lock();
		if (networkManager) {
			networkManager->WSClose(interfaceName);
		}
	}

	bool NetworkInterfaceWS::Connect(std::string_view interfaceName,
		int32_t retries,
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb) {

		bool result = false;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto query = it->second.GetQuery(interfaceName);
				if (!query.empty()) {
					auto networkManager = mNetworkManager.lock();
					if (networkManager) {
						result = networkManager->PostQuery(interfaceName, "", retries, query, expectedResponseSize, timeOut, cb);
					}
				}
			}
		}
		return result;
	}

	int32_t NetworkInterfaceWS::Read(std::string_view interfaceName, char* buffer, size_t size) {
		int32_t read = 0;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto networkManager = mNetworkManager.lock();
				if (networkManager) {
					read = networkManager->WSRead(interfaceName, buffer, size);
				}
			}
		}
		return read;
	}

	void NetworkInterfaceWS::WriteQueued(std::string_view interfaceName, int32_t maxQueued) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto networkManager = mNetworkManager.lock();
				if (networkManager) {
					auto& queue = it->second.GetQueue();
					while (!queue.empty() && maxQueued > 0) {
						auto& command = queue.front();
						auto written = networkManager->WSWrite(interfaceName, command.c_str(), command.size());
						if (written > 0) {
							queue.pop_front();
						}
						else {
							LOG(LogWarning) << "Failed to write to: " << interfaceName << " : command: " << command;
						}
						maxQueued--;
					}
				}
			}
		}
	}

	void NetworkInterfaceWS::QueueWrite(std::string_view interfaceName, const char* buffer, size_t size) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto& queue = it->second.GetQueue();
				queue.push_back(std::string(buffer, size));
			}
		}
	}

	int32_t NetworkInterfaceWS::Write(std::string_view interfaceName, const char* buffer, size_t size) {
		int32_t written = false;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto networkManager = mNetworkManager.lock();
				if (networkManager) {
					written = networkManager->WSWrite(interfaceName, buffer, size) >= 0;
					if (written < 0) {
						LOG(LogWarning) << "Failed to write to: " << interfaceName << " : error: " << written;
					}
				}
			}
		}
		return written;
	}

	bool NetworkInterfaceWS::IsConnected(std::string_view interfaceName) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			auto networkManager = mNetworkManager.lock();
			if (networkManager) {
				return networkManager->WSConnected(interfaceName);
			}
		}
		return false;
	}

	bool NetworkInterfaceWS::NetworkStatus(std::string_view interfaceName) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			return it->second.Status();
		}
		return false;
	}

	void NetworkInterfaceWS::SetNetworkStatus(std::string_view interfaceName, bool isup) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			it->second.SetStatus(isup);
		}
	}
}
