#pragma once

#include "network/NetworkManager.h"
#include "network/NetworkHostInfo.h"

#include <cstdint>
#include <memory>
#include <filesystem>
#include <map>
#include <functional>
#include <deque>

namespace l::network {

	class NetworkInterfaceWS {
	public:
		NetworkInterfaceWS(std::weak_ptr<l::network::NetworkManager> networkManager) :
			mNetworkManager(networkManager)
		{}
		~NetworkInterfaceWS() {}

		void CreateInterface(std::string_view interfaceName, 
			std::string_view protocol, 
			std::string_view host, 
			uint32_t port = 0,
			int32_t networkStatusInterval = 20);
		void Shutdown();
		bool Connect(std::string_view interfaceName,
			int32_t retries = 3,
			int32_t expectedResponseSize = 0,
			int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr);
		void Disconnect(std::string_view queryName);
		int32_t Read(std::string_view interfaceName, char* buffer, size_t size);
		void QueueWrite(std::string_view interfaceName, const char* buffer, size_t size);
		int32_t Write(std::string_view interfaceName, const char* buffer, size_t size);
		void SendQueued(std::string_view interfaceName, int32_t maxQueued);
		int32_t NumQueued(std::string_view interfaceName);
		void ClearQueued(std::string_view interfaceName);

		bool IsConnected(std::string_view interfaceName);

		bool NetworkStatus(std::string_view interfaceName);

		void CreateWebSocket(
			std::string_view interfaceName,
			std::string_view endpointString,
			std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, l::network::WebSocket&)> handler = nullptr
		) {
			auto network = mNetworkManager.lock();
			if (network) {
				auto it = mInterfaces.find(interfaceName.data());
				if (it != mInterfaces.end()) {
					it->second.AddEndpoint(interfaceName, endpointString);
					network->CreateRequest(std::make_unique<l::network::WebSocket>(interfaceName, "", 0, handler));
				}
			}
		}

	protected:
		void SetNetworkStatus(std::string_view interfaceName, bool isup);

		std::weak_ptr<l::network::NetworkManager> mNetworkManager;
		std::map<std::string, HostInfo> mInterfaces;
	};

	std::shared_ptr<NetworkInterfaceWS> CreateNetworkInterfaceWS(std::weak_ptr<l::network::NetworkManager> networkManager);

}