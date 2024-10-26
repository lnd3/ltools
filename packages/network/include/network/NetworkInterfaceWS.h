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
		bool Connect(std::string_view interfaceName, std::string_view queryName);
		void Disconnect(std::string_view interfaceName, std::string_view queryName);
		bool Send(std::string_view interfaceName,
			std::string_view queryName,
			std::string_view arguments = "",
			int32_t retries = 3,
			int32_t expectedResponseSize = 0,
			int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr);
		bool NetworkStatus(std::string_view interfaceName);

		template<class T>
		void CreateWebSocketTemplate(
			std::string_view interfaceName,
			std::string_view queryName,
			std::string_view endpointString,
			int32_t numMaxParallellRequestConnections,
			int32_t defaultResponseSize,
			int32_t timeout,
			std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, l::network::Request<T>&)> handler = nullptr
		) {
			auto network = mNetworkManager.lock();
			if (network) {
				auto it = mInterfaces.find(interfaceName.data());
				if (it != mInterfaces.end()) {
					it->second.AddEndpoint(queryName, endpointString);

					if (handler) {
						auto handlerWrapped = [&, cb = handler, name = std::string(interfaceName)](bool success, std::string_view args, l::network::Request<T>& request) {
							SetNetworkStatus(name, success);
							return cb(success, args, request);
							};
						for (int i = 0; i < numMaxParallellRequestConnections; i++) {
							network->CreateRequestTemplate(std::make_unique<l::network::Request<T>>(queryName, "", defaultResponseSize, handlerWrapped, timeout));
						}
					}
					else {
						for (int i = 0; i < numMaxParallellRequestConnections; i++) {
							network->CreateRequestTemplate(std::make_unique<l::network::Request<T>>(queryName, "", defaultResponseSize, nullptr, timeout));
						}
					}
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