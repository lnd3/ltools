#pragma once

#include "network/NetworkManager.h"

#include <cstdint>
#include <memory>
#include <filesystem>
#include <map>
#include <functional>

namespace l::network {

	class HostInfo {
	public:
		HostInfo(std::string_view protocol, std::string_view host, uint32_t port = 0, int32_t networkStatusInterval = 20) :
			mProtocol(protocol), 
			mHost(host),
			mPort(port),
			mIsHostResponding(true),
			mLastStatusCheck(0),
			mNetworkStatusInterval(networkStatusInterval)
		{}
		~HostInfo() {};

		HostInfo(const HostInfo& other) {
			this->mProtocol = other.mProtocol;
			this->mHost = other.mHost;
			this->mPort = other.mPort;
			this->mRequestQueries = other.mRequestQueries;
			this->mIsHostResponding.store(other.mIsHostResponding.load());
			this->mLastStatusCheck.store(other.mLastStatusCheck.load());
			this->mNetworkStatusInterval = other.mNetworkStatusInterval;
		}

		bool Status();
		void SetStatus(bool isup);

		void AddEndpoint(std::string_view queryName, std::string_view endpointString);
		std::string GetQuery(std::string_view queryName = "", std::string_view arguments = "");

	protected:
		std::string mProtocol; // http, https etc
		std::string mHost; // host url/server etc
		uint32_t mPort; // 8080 etc
		std::map<std::string, std::string> mRequestQueries; // endpoint string
		std::atomic_bool mIsHostResponding;
		std::atomic_int32_t mLastStatusCheck;
		int32_t mNetworkStatusInterval;
	};

	class NetworkInterface {
	public:
		NetworkInterface(std::weak_ptr<l::network::NetworkManager> networkManager) :
			mNetworkManager(networkManager)
		{}
		~NetworkInterface() {}

		void CreateInterface(std::string_view interfaceName, 
			std::string_view protocol, 
			std::string_view host, 
			uint32_t port = 0,
			int32_t networkStatusInterval = 20);
		bool SendRequest(std::string_view interfaceName, 
			std::string_view requestName, 
			std::string_view arguments, 
			int32_t retries, 
			int32_t expectedResponseSize,
			int32_t timeOut,
			std::function<void(bool, std::string_view)> cb = nullptr);
		bool NetworkStatus(std::string_view interfaceName);

		template<class T>
		void CreateRequestTemplate(
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

	std::shared_ptr<NetworkInterface> CreateNetworkInterface(std::weak_ptr<l::network::NetworkManager> networkManager);

}