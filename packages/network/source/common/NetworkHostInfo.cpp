#include "network/NetworkHostInfo.h"

#include "logging/String.h"

#include <memory>

namespace l::network {

	bool HostInfo::Status() {
		if (mNetworkStatusInterval == 0 || mIsHostResponding) {
			return true;
		}
		auto now = l::string::get_unix_epoch();
		if (mLastStatusCheck < now) {
			// every x seconds we allow one connection through to test connectivity
			mLastStatusCheck = now + mNetworkStatusInterval;
			return true;
		}
		return false;
	}

	void HostInfo::SetStatus(bool isup) {
		mLastStatusCheck = l::string::get_unix_epoch() + 20;
		mIsHostResponding = isup;
	}

	void HostInfo::AddEndpoint(std::string_view queryName, std::string_view endpointString) {
		mRequestQueries.emplace(queryName, endpointString);
	}

	std::string HostInfo::GetQuery(std::string_view queryName, std::string_view arguments) {
		std::stringstream query;
		query << mProtocol;
		query << "://";
		query << mHost;
		ASSERT(mHost.at(mHost.size() - 1) != '/');
		if (mPort > 0) {
			query << ":";
			query << std::to_string(mPort);
		}
		if (!queryName.empty()) {
			auto it = mRequestQueries.find(queryName.data());
			if (it != mRequestQueries.end()) {
				if (!it->second.empty()) {
					if (it->second.at(0) != '/') {
						query << "/";
					}
					query << it->second;
				}
				if (!arguments.empty()) {
					query << arguments;
				}
			}
		}
		return query.str();
	}

	std::deque<std::string>& HostInfo::GetQueue() {
		return mWriteQueue;
	}

}
