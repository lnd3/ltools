#pragma once

#include <math.h>
#include <string>
#include <sstream>
#include <functional>
#include <vector>

#include "curl/curl.h"

#ifndef CURLPIPE_MULTIPLEX
#define CURLPIPE_MULTIPLEX 0
#endif

#include "logging/LoggingAll.h"
#include "concurrency/ExecutorService.h"
#include "network/NetworkConnection.h"

namespace l::network {

	class NetworkManager {
	public:
		NetworkManager(int numThreads, bool multiplex) : mPostedRequests(0) {
			Startup(numThreads, multiplex);
		}

		~NetworkManager() {
			Shutdown();
		}

		void Startup(int numThreads, bool multiplex);
		void ClearJobs();
		void Shutdown();

		int32_t ActiveRequests();
		int32_t TotalRequestCount();
		int32_t SentRequestCount();
		int32_t CompletedRequestCount();
		void ToggleVerboseLogging();

		bool CreateRequestTemplate(std::unique_ptr<ConnectionBase> request);
		bool PostQuery(std::string_view queryName,
			std::string_view queryArguments, 
			int32_t maxTries = 3, 
			std::string_view query = "",
			int32_t expectedResponseSize = 0,
			int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr);

		void WSClose(std::string_view queryName = "");
		int32_t WSWrite(std::string_view queryName, char* buffer, size_t size);
		int32_t WSRead(std::string_view queryName, char* buffer, size_t size);

	protected:
		std::thread mCurlPerformer;
		std::mutex mConnectionsMutex;
		std::vector<std::unique_ptr<ConnectionBase>> mConnections;
		std::unique_ptr<l::concurrency::ExecutorService> mJobManager;

		int32_t mPostedRequests;

		CURLM* mMultiHandle = nullptr;
	};

	std::shared_ptr<NetworkManager> CreateNetworkManager(int numThreads, bool multiplex);

}
