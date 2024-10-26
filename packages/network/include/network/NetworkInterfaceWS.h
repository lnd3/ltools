#pragma once

#include "network/NetworkManager.h"
#include "network/NetworkHostInfo.h"

#include <cstdint>
#include <memory>
#include <filesystem>
#include <map>
#include <functional>

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
			std::string_view queryName,
			std::string_view arguments = "",
			int32_t retries = 3,
			int32_t expectedResponseSize = 0,
			int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr);
		void Disconnect(std::string_view queryName);
		int32_t Read(std::string_view interfaceName, std::string_view queryName, char* buffer, size_t size);
		bool Write(std::string_view interfaceName, std::string_view queryName, char* buffer, size_t size);

		bool NetworkStatus(std::string_view interfaceName);

		template<class T>
		void CreateWebSocketTemplate(
			std::string_view interfaceName,
			std::string_view queryName,
			std::string_view endpointString,
			std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, l::network::Request<T>&)> handler = nullptr
		) {
			auto network = mNetworkManager.lock();
			if (network) {
				auto it = mInterfaces.find(interfaceName.data());
				if (it != mInterfaces.end()) {
					it->second.AddEndpoint(queryName, endpointString);
					network->CreateRequestTemplate(std::make_unique<l::network::Request<T>>(queryName, "", 0, handler));
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