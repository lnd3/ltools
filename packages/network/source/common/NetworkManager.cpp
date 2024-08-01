#pragma once

#include "network/NetworkManager.h"

namespace l::network {

	std::shared_ptr<NetworkManager> CreateNetworkManager(int numThreads, bool multiplex) {
		return std::make_shared<NetworkManager>(numThreads, multiplex);
	}

	size_t CurlClientWriteCallback(char* contents, size_t size, size_t nmemb, void* userp) {
		auto request = reinterpret_cast<RequestBase*>(userp);
		if (request != nullptr) {
			request->AppendResponse(contents, size * nmemb);
			return size * nmemb;
		}
		return size * nmemb;
	}

	size_t CurlClientProgressCallback(void* clientp, double dltotal, double dlnow, double, double) {
		auto progress = reinterpret_cast<Progress*>(clientp);
		double progressTotal = dlnow / dltotal;
		LOG(LogDebug) << "downloaded " << progress->size << " bytes" << " and progress at " << std::to_string(int(progressTotal * 100)) << "%";
		return 0;
	}

	void NetworkManager::ClearJobs() {
		mJobManager->clearJobs();
	}

	void NetworkManager::Shutdown() {
		if (mJobManager) {
			mJobManager->shutdown();
			mJobManager.reset();

			std::lock_guard lock(mConnectionsMutex);
			mConnections.clear();

			if (mMultiHandle != nullptr && mCurlPerformer.joinable()) {
				mCurlPerformer.join();
			}
			if (mMultiHandle != nullptr) {
				curl_multi_cleanup(mMultiHandle);
			}
			curl_global_cleanup();
		}
	}

	int32_t NetworkManager::ActiveRequests() {
		if (!mJobManager) {
			return 0;
		}
		return mJobManager->numJobs();
	}

	int32_t NetworkManager::TotalRequestCount() {
		if (!mJobManager) {
			return 0;
		}
		return mJobManager->numTotalJobs();
	}

	int32_t NetworkManager::SentRequestCount() {
		return mPostedRequests;
	}

	int32_t NetworkManager::CompletedRequestCount() {
		if (!mJobManager) {
			return 0;
		}
		return mJobManager->numCompletedJobs();
	}

	void NetworkManager::ToggleVerboseLogging() {
		mJobManager->gDebugLogging = !mJobManager->gDebugLogging;
	}

	bool NetworkManager::CreateRequestTemplate(std::unique_ptr<RequestBase> request) {
		std::lock_guard lock(mConnectionsMutex);
		mConnections.emplace_back(std::move(request));
		return true;
	}

	bool NetworkManager::PostQuery(std::string_view requestName, 
		std::string_view queryArguments, 
		int32_t maxTries, 
		std::string_view query, 
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb) {
		if (!mJobManager) {
			return false;
		}
		return mJobManager->queueJob(
			std::make_unique<l::concurrency::Worker>(requestName, [=, 
				requestName = std::string(requestName), 
				queryArguments = std::string(queryArguments),
				query = std::string(query),
				callback = cb
			](const l::concurrency::RunState& state
					) {
				std::unique_lock lock(mConnectionsMutex);
				auto it = std::find_if(mConnections.begin(), mConnections.end(), [&](std::unique_ptr<RequestBase>& request) {
					if (requestName != request->GetRequestName()) {
						return false;
					}
					if(request->TryReservingRequest()) {
						return true;
					}
					return false;
					});

				if (it == mConnections.end()) {
					return l::concurrency::RunnableResult::REQUEUE_DELAYED;
				}
				auto request = it->get();
				lock.unlock();

				auto result = request->SendAndUnReserveRequest(mMultiHandle, state, queryArguments, query, expectedResponseSize, timeOut, callback);
				if (result != l::concurrency::RunnableResult::SUCCESS) {
					mPostedRequests++;
				}
				return result;
				},
				maxTries));
	}
}
