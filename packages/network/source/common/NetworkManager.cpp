#include "network/NetworkManager.h"

namespace l::network {

	std::shared_ptr<NetworkManager> CreateNetworkManager(int numThreads, bool multiplex) {
		return std::make_shared<NetworkManager>(numThreads, multiplex);
	}

	void NetworkManager::Startup(int numThreads, bool multiplex) {
		CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
		ASSERT(res == CURLE_OK) << "Failed to init curl global";

		mJobManager = std::make_unique< l::concurrency::ExecutorService>("NetworkManager", numThreads);
		mJobManager->startJobs();

		mMultiHandle = nullptr;
		if (multiplex) {
			mMultiHandle = curl_multi_init();
		}

		if (mMultiHandle != nullptr) {
			curl_multi_setopt(mMultiHandle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
			mCurlPerformer = std::thread([&]() {
				int32_t runningHandles = 0;
				CURLMcode mc;
				do {
					mc = curl_multi_perform(mMultiHandle, &runningHandles);
					if (mc == CURLM_OK) {
						int32_t messagesInQueue;
						struct CURLMsg* m;
						do {
							m = curl_multi_info_read(mMultiHandle, &messagesInQueue);
							if (m && (m->msg == CURLMSG_DONE)) {
								CURL* e = m->easy_handle;
								bool success = true;
								if (m->data.result != CURLE_OK) {
									success = false;
								}
								bool foundHandle = false;
								for (auto& it : mConnections) {
									if (it->IsHandle(e)) {
										foundHandle = true;
										if (!it->IsWebSocket()) {
											it->NotifyCompleteRequest(success);
										}
									}
								}
								ASSERT(foundHandle);
							}
							else if (m) {
								LOG(LogWarning) << "Not done";
							}
						} while (m != nullptr && messagesInQueue > 0);

						int numfds;
						mc = curl_multi_poll(mMultiHandle, NULL, 0, 1000, &numfds);
						if (mc != CURLM_OK) {
							LOG(LogError) << "curl_multi_poll failed, code " << mc;
						}
					}
					else {
						LOG(LogError) << "curl_multi_perform failed, code " << mc;
					}
				} while (runningHandles > 0 || mJobManager.get() != nullptr || !mConnections.empty());
				});
		}
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

	bool NetworkManager::CreateRequest(std::unique_ptr<ConnectionBase> request) {
		std::lock_guard lock(mConnectionsMutex);
		mConnections.emplace_back(std::move(request));
		return true;
	}

	bool NetworkManager::PostQuery(std::string_view queryName,
		std::string_view queryArguments, 
		int32_t maxTries, 
		std::string_view query, 
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb) {
		if (!mJobManager) {
			return false;
		}

		auto job = [
				cqueryName = std::string(queryName),
				cqueryArguments = std::string(queryArguments),
				cquery = std::string(query),
				cexpectedResponseSize = expectedResponseSize,
				ctimeOut = timeOut,
				ccallback = cb,
				&cmConnectionsMutex = mConnectionsMutex,
				&cmConnections = mConnections,
				&cmMultiHandle = mMultiHandle,
				&cmPostedRequests = mPostedRequests
			](const l::concurrency::RunState& state) {
				std::unique_lock lock(cmConnectionsMutex);
				auto it = std::find_if(cmConnections.begin(), cmConnections.end(), [&](std::unique_ptr<ConnectionBase>& request) {
					if (cqueryName != request->GetRequestName()) {
						return false;
					}
					if (request->TryReservingRequest()) {
						return true;
					}
					return false;
					});

				if (it == cmConnections.end()) {
					return l::concurrency::RunnableResult::REQUEUE_DELAYED;
				}
				auto request = it->get();
				lock.unlock();

				auto result = request->SendAndUnReserveRequest(cmMultiHandle, state, cqueryArguments, cquery, cexpectedResponseSize, ctimeOut, ccallback);
				if (result != l::concurrency::RunnableResult::SUCCESS) {
					cmPostedRequests++;
				}
				return result;
			};

		auto work = std::make_unique<l::concurrency::Worker>(queryName, std::move(job), maxTries);

		return mJobManager->queueJob(std::move(work));
	}

	void NetworkManager::WSClose(std::string_view queryName) {
		std::unique_lock lock(mConnectionsMutex);

		if (queryName.empty()) {
			for (auto& c : mConnections) {
				if (c->IsWebSocket()) {
					c->WSClose();
				}
			}
			lock.unlock();
		}
		else {
			auto it = std::find_if(mConnections.begin(), mConnections.end(), [&](std::unique_ptr<ConnectionBase>& request) {
				if (queryName != request->GetRequestName()) {
					return false;
				}
				if (!request->HasExpired()) {
					return true;
				}
				return false;
				});

			if (it == mConnections.end()) {
				return;
			}
			auto request = it->get();
			lock.unlock();

			request->WSClose();
		}
	}

	int32_t NetworkManager::WSWrite(std::string_view queryName, char* buffer, size_t size) {
		std::unique_lock lock(mConnectionsMutex);
		auto it = std::find_if(mConnections.begin(), mConnections.end(), [&](std::unique_ptr<ConnectionBase>& request) {
			if (queryName == request->GetRequestName()) {
				return true;
			}
			return false;
			});

		if (it == mConnections.end()) {
			LOG(LogError) << "Failed wss write, query not found";
			return -4;
		}
		auto request = it->get();
		lock.unlock();

		return request->WSWrite(buffer, size);
	}

	int32_t NetworkManager::WSRead(std::string_view queryName, char* buffer, size_t size) {
		std::unique_lock lock(mConnectionsMutex);
		auto it = std::find_if(mConnections.begin(), mConnections.end(), [&](std::unique_ptr<ConnectionBase>& request) {
			if (queryName == request->GetRequestName()) {
				return true;
			}
			return false;
			});

		if (it == mConnections.end()) {
			LOG(LogError) << "Failed wss read, query not found";
			return -4;
		}
		auto request = it->get();
		lock.unlock();

		return request->WSRead(buffer, size);
	}

}
