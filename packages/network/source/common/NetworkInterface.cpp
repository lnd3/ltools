#include "network/NetworkInterface.h"

#include "logging/String.h"

#include <memory>

namespace l::network {

	std::shared_ptr<NetworkInterface> CreateNetworkInterface(std::weak_ptr<l::network::NetworkManager> networkManager) {
		return std::make_shared<NetworkInterface>(networkManager);
	}

	bool HostInfo::Status() {
		if (mNetworkStatusInterval == 0 || mIsHostResponding) {
			return true;
		}
		auto now = l::string::get_unix_timestamp();
		if (mLastStatusCheck < now) {
			// every x seconds we allow one connection through to test connectivity
			mLastStatusCheck = now + mNetworkStatusInterval;
			return true;
		}
		return false;
	}

	void HostInfo::SetStatus(bool isup) {
		mLastStatusCheck = l::string::get_unix_timestamp() + 20;
		mIsHostResponding = isup;
	}

	void HostInfo::AddEndpoint(std::string_view queryName, std::string_view endpointString) {
		mRequestQueries.emplace(queryName, endpointString);
	}

	std::string HostInfo::GetQuery(std::string_view requestName, std::string_view arguments) {
		std::stringstream query;
		query << mProtocol;
		query << "://";
		query << mHost;
		ASSERT(mHost.at(mHost.size() - 1) != '/');
		if (mPort > 0) {
			query << ":";
			query << std::to_string(mPort);
		}
		if (!requestName.empty()) {
			auto it = mRequestQueries.find(requestName.data());
			if (it != mRequestQueries.end()) {
				if (it->second.at(0) != '/') {
					query << "/";
				}
				query << it->second;
				if (!arguments.empty()) {
					query << arguments;
				}
			}
		}
		return query.str();
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

			std::string requestName = interfaceName.data();
			requestName += "Ping";
			CreateRequestTemplate<std::stringstream>(interfaceName, requestName, "", 1, 5000, 3, pingHandler);
		}
	}

	bool NetworkInterface::SendRequest(std::string_view interfaceName, 
		std::string_view requestName,
		std::string_view queryArguments, 
		int32_t retries, 
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb) {

		bool result = false;
		auto it = mInterfaces.find(interfaceName.data());
		if (it != mInterfaces.end()) {
			if (NetworkStatus(interfaceName)) {
				auto query = it->second.GetQuery(requestName, queryArguments);
				if (!query.empty()) {
					auto networkManager = mNetworkManager.lock();
					if (networkManager) {
						result = networkManager->PostQuery(requestName, queryArguments, retries, query, expectedResponseSize, timeOut, cb);
					}
				}
			}
		}
		return result;
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
