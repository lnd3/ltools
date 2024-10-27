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

	void NetworkInterfaceWS::Disconnect(std::string_view queryName) {
		auto networkManager = mNetworkManager.lock();
		if (networkManager) {
			networkManager->WSClose(queryName);
		}
	}

	bool NetworkInterfaceWS::Connect(std::string_view interfaceName,
		std::string_view queryName,
		std::string_view queryArguments,
		int32_t retries,
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb) {

		bool result = false;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto query = it->second.GetQuery(queryName, queryArguments);
				if (!query.empty()) {
					auto networkManager = mNetworkManager.lock();
					if (networkManager) {
						result = networkManager->PostQuery(queryName, queryArguments, retries, query, expectedResponseSize, timeOut, cb);
					}
				}
			}
		}
		return result;
	}

	int32_t NetworkInterfaceWS::Read(std::string_view interfaceName, std::string_view queryName, char* buffer, size_t size) {
		int32_t read = 0;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto networkManager = mNetworkManager.lock();
				if (networkManager) {
					read = networkManager->WSRead(queryName, buffer, size);
				}
			}
		}
		return read;
	}

	bool NetworkInterfaceWS::Write(std::string_view interfaceName, std::string_view queryName, char* buffer, size_t size) {
		bool result = false;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto networkManager = mNetworkManager.lock();
				if (networkManager) {
					result = networkManager->WSWrite(queryName, buffer, size) == 0;
				}
			}
		}
		return result;
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
