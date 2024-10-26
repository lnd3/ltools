#pragma once

#include <cstdint>
#include <memory>
#include <filesystem>
#include <map>
#include <functional>
#include <atomic>

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

}