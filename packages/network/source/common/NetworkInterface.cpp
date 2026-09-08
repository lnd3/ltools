#include "network/NetworkInterface.h"

#include "logging/String.h"

#include <memory>

namespace l::network {

	std::shared_ptr<NetworkInterface> CreateNetworkInterface(std::weak_ptr<l::network::NetworkManager> networkManager) {
		return std::make_shared<NetworkInterface>(networkManager);
	}

	void NetworkInterface::CreateInterface(std::string_view interfaceName, 
		std::string_view protocol, 
		std::string_view host, 
		uint32_t port, 
		int32_t networkStatusInterval) {
		if (!mInterfaces.contains(interfaceName.data())) {
			auto pingHandler = [&, name = std::string(interfaceName)](bool success, std::string_view, l::network::RequestStringStream&) {
				SetNetworkStatus(name, success);
				if (success) {
					return l::concurrency::RunnableResult::SUCCESS;
				}
				return l::concurrency::RunnableResult::FAILURE;
				};

			mInterfaces.emplace(std::string(interfaceName), HostInfo(protocol, host, port, networkStatusInterval) );

			std::string queryName = interfaceName.data();
			queryName += "Ping";
			CreateRequest<std::stringstream>(interfaceName, queryName, "", 1, 5000, 3, pingHandler);
		}
	}

	void NetworkInterface::Shutdown() {

	}

	bool NetworkInterface::SendRequest(std::string_view interfaceName,
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

	bool NetworkInterface::SendJsonRequest(std::string_view interfaceName,
		std::string_view endpointPath,
		std::string_view jsonBody,
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb) {

		auto it = mInterfaces.find(interfaceName.data());
		if (it == mInterfaces.end()) {
			return false;
		}

		// Register endpoint on first use (idempotent — AddEndpoint uses emplace)
		it->second.AddEndpoint(endpointPath.data(), endpointPath);
		auto query = it->second.GetQuery(endpointPath);
		if (query.empty()) {
			return false;
		}

		auto networkManager = mNetworkManager.lock();
		if (!networkManager) {
			return false;
		}

		// Use the endpoint path as the query name so the connection pool
		// can be keyed per-endpoint (/exchange vs /info)
		return networkManager->PostQuery(
			endpointPath,   // queryName (matches pre-created connection slot)
			"",             // queryArguments (unused for POST)
			1,              // maxTries
			query,          // full URL
			expectedResponseSize,
			timeOut,
			cb,
			jsonBody,
			{}
		);
	}

	bool NetworkInterface::NetworkStatus(std::string_view interfaceName) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			return it->second.Status();
		}
		return false;
	}

	void NetworkInterface::SetNetworkStatus(std::string_view interfaceName, bool isup) {
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			it->second.SetStatus(isup);
		}
	}
}
