#include "network/NetworkConnection.h"

namespace l::network {

	int CurlClientCloseSocket(void* userdata, curl_socket_t item) {
		auto request = reinterpret_cast<ConnectionBase*>(userdata);
		if (request != nullptr) {
			request->NotifyClose();
		}
		LOG(LogDebug) << "Socket close: " << item;
		return 0;
	}

	size_t CurlClientWriteHeader(char* contents, size_t size, size_t nitems, void* userdata) {
		auto request = reinterpret_cast<ConnectionBase*>(userdata);
		if (request != nullptr) {
			request->NotifyAppendHeader(contents, size * nitems);
		}
		return size * nitems;
	}

	size_t CurlClientWriteCallback(char* contents, size_t size, size_t nmemb, void* userdata) {
		auto request = reinterpret_cast<ConnectionBase*>(userdata);
		if (request != nullptr) {
			if (request->IsWebSocket()) {
				auto wssMeta = request->GetWebSocketMeta();
				if (wssMeta) {
					LOG(LogDebug) << "wss meta flags " << wssMeta->flags;
					LOG(LogDebug) << "wss meta length " << wssMeta->len;
				}
				request->NotifyAppendResponse(contents, size * nmemb);
			}
			else {
				request->NotifyAppendResponse(contents, size * nmemb);
			}
		}
		return size * nmemb;
	}

	size_t CurlClientProgressCallback(void* clientp, double dltotal, double dlnow, double, double) {
		auto progress = reinterpret_cast<Progress*>(clientp);
		double progressTotal = dlnow / dltotal;
		LOG(LogDebug) << "downloaded " << progress->size << " bytes" << " and progress at " << std::to_string(int(progressTotal * 100)) << "%";
		return 0;
	}



	std::string_view ConnectionBase::GetRequestName() {
		return mName;
	}

	bool ConnectionBase::TryReservingRequest() {
		bool available = false;
		if (mCompletedRequest && mOngoingRequest.compare_exchange_strong(available, true)) {
			mCompletedRequest = false;
			mStarted = l::string::get_unix_epoch_ms();
			return true;
		}
		return false;
	}

	l::concurrency::RunnableResult ConnectionBase::SendAndUnReserveRequest(
		CURLM* multiHandle,
		const l::concurrency::RunState& state,
		const std::string& queryArguments,
		const std::string& query,
		int32_t expectedResponseSize,
		int32_t timeOut,
		std::function<void(bool, std::string_view)> cb
	) {
		ASSERT(mOngoingRequest) << "Request has not been reserved for usage";
		ASSERT(!mCompletedRequest);

		if (mRequestQuery.empty() && query.empty()) {
			mOngoingRequest = false;
			return l::concurrency::RunnableResult::CANCELLED;
		}

		if (!mCurl) {
			mCurl = curl_easy_init();
		}
		if (!mCurl) {
			return l::concurrency::RunnableResult::FAILURE;
		}

		Progress progress{};
		progress.state = &state;

		if (!query.empty()) {
			mRequestQuery = query;
		}
		if (l::string::equal(mRequestQuery.c_str(), "ws", 0, 0, 2)) {
			mIsWebSocket = true;
		}

		mRequestQueryArgs = queryArguments;
		mResponseSize = 0;

		SetResponseSize(expectedResponseSize);

		curl_easy_setopt(mCurl, CURLOPT_CONNECT_ONLY, mIsWebSocket ? 2L : 0L);
		curl_easy_setopt(mCurl, CURLOPT_URL, mRequestQuery.c_str());
		LOG(LogDebug) << mRequestQuery;
		//curl_easy_setopt(mCurl, CURLOPT_FRESH_CONNECT, 0L); // only use if necessary to create a new connection
		auto res_verify_peer = curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		ASSERT(res_verify_peer == CURLE_OK) << "Failed to verify peer: " << std::to_string(res_verify_peer);
		auto res_verify_host = curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, 0L);
		ASSERT(res_verify_host == CURLE_OK) << "Failed to verify host: " << std::to_string(res_verify_host);
		auto res_ssl = curl_easy_setopt(mCurl, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_ALLOW_BEAST | CURLSSLOPT_NO_REVOKE);
		ASSERT(res_ssl == CURLE_OK) << "Failed to set ssl options: " << std::to_string(res_ssl);

		//curl_easy_setopt(mCurl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(mCurl, CURLOPT_CLOSESOCKETFUNCTION, CurlClientCloseSocket);
		curl_easy_setopt(mCurl, CURLOPT_CLOSESOCKETDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_HEADERFUNCTION, CurlClientWriteHeader);
		curl_easy_setopt(mCurl, CURLOPT_HEADERDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, CurlClientWriteCallback);
		curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_XFERINFODATA, &progress);
		curl_easy_setopt(mCurl, CURLOPT_XFERINFOFUNCTION, CurlClientProgressCallback);
		curl_easy_setopt(mCurl, CURLOPT_TIMEOUT, timeOut >= 0 ? timeOut : mTimeout.load());

		curl_easy_setopt(mCurl, CURLOPT_BUFFERSIZE, mDefaultResponseSize);

		mStarted = l::string::get_unix_epoch_ms();
		mSuccess = true;

		if (multiHandle != nullptr) {
			curl_easy_setopt(mCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
#if (CURLPIPE_MULTIPLEX > 0)
			/* wait for pipe connection to confirm */
			curl_easy_setopt(mCurl, CURLOPT_PIPEWAIT, 1L);
#endif
			auto curlMCode = curl_multi_add_handle(multiHandle, mCurl);
			if (curlMCode != CURLMcode::CURLM_OK) {
				LOG(LogError) << "Curl failure  " << std::to_string(curlMCode) << ": " << mRequestQueryArgs;
				mSuccess = false;
			}
		}
		else {
			auto curlCode = curl_easy_perform(mCurl);
			//curl_easy_header()
			if (curlCode != CURLE_OK) {
				LOG(LogError) << "Curl failure  " << std::to_string(curlCode) << ": " << mRequestQueryArgs;
				mSuccess = false;
			}
		}

		if (IsWebSocket() || multiHandle != nullptr) {
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (state.mDestructing || HasExpired()) {
					bool completed = false;
					if (mCompletedRequest.compare_exchange_strong(completed, true)) {
						mSuccess = false;
					}
				}
			} while (!mCompletedRequest);
		}
		else {
			// notify manually since easy perform blocks until completed for single connections mode
			NotifyCompleteRequest(mSuccess);
		}

		l::concurrency::RunnableResult result = mSuccess ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;

		if (CallHandler() != l::concurrency::RunnableResult::SUCCESS) {
			result = l::concurrency::RunnableResult::FAILURE;
		}

		if (cb) {
			cb(result == l::concurrency::RunnableResult::SUCCESS, mRequestQueryArgs);
		}

		if (multiHandle != nullptr) {
			curl_multi_remove_handle(multiHandle, mCurl);
		}
		ASSERT(mCompletedRequest);

		mOngoingRequest = false;
		return result;
	}

	void ConnectionBase::NotifyCompleteRequest(bool success) {
		bool completed = false;
		if (mOngoingRequest && mCompletedRequest.compare_exchange_strong(completed, true)) {
			mSuccess = success;
			return;
		}
	}

	bool ConnectionBase::IsHandle(CURL* handle) {
		if (mCurl == handle) {
			return true;
		}
		return false;
	}

	bool ConnectionBase::HasExpired() {
		if (mOngoingRequest && mTimeout > 0) {
			auto timeWaitingMs = static_cast<int32_t>(l::string::get_unix_epoch_ms() - mStarted);
			auto expired = timeWaitingMs > mTimeout * 1000;
			return expired;
		}
		return false;
	}

	void ConnectionBase::NotifyClose() {

	}

	bool ConnectionBase::IsWebSocket() {
		return mIsWebSocket;
	}

	const curl_ws_frame* ConnectionBase::GetWebSocketMeta() {
		const struct curl_ws_frame* m = curl_ws_meta(mCurl);
		return m;
	}

	int32_t ConnectionBase::WSWrite(const char* buffer, size_t size) {
		if (HasExpired()) {
			LOG(LogError) << "Failed wss write, connection expired";
			return -2;
		}
		if (mCurl == nullptr) {
			LOG(LogError) << "Failed wss write, no curl instance";
			return -3;
		}
		size_t sentBytes = 0;
		auto res = curl_ws_send(mCurl, buffer, size, &sentBytes, 0, CURLWS_TEXT);
		if (res != CURLE_OK) {
			LOG(LogError) << "Failed wss write, error: " << res;
		}
		return res == CURLE_OK ? static_cast<int32_t>(sentBytes) : -1;
	}

	int32_t ConnectionBase::WSRead(char* buffer, size_t size) {
		if (HasExpired()) {
			LOG(LogError) << "Failed wss read, connection expired";
			return -2;
		}
		if (mCurl == nullptr) {
			LOG(LogError) << "Failed wss read, no curl instance";
			return -3;
		}
		const struct curl_ws_frame* meta;
		size_t readBytes = 0;
		auto res = curl_ws_recv(mCurl, buffer, size, &readBytes, &meta);
		if (res == CURLE_AGAIN || res == CURLE_OK){
			return static_cast<int32_t>(readBytes);
		}
		//LOG(LogError) << "Failed wss read, error: " << res;
		return -1;
	}

	void ConnectionBase::WSClose() {
		NotifyCompleteRequest(true);
	}

	void ConnectionBase::NotifyAppendHeader(const char* contents, size_t size) {
		ASSERT(mOngoingRequest);

		if (mCompletedRequest) {
			// request probably timed out so discard data
			return;
		}

		const std::string header = std::string(contents, size);

		mHeaders.push_back(header);

		auto arr = l::string::split(header, ":\n\r");
		if (arr.size() == 1) {
			auto& s = arr.at(0);
			auto i = s.find_first_of(' ');
			if (i == s.size()) {
				mHeaderMap.emplace(arr.at(0), "");
			}
			else {
				mHeaderMap.emplace(s.substr(0, i), s.substr(i + 1, s.size() - i));
			}
		}
		else if (arr.size() >= 2) {
			auto& d = arr.at(1);
			mHeaderMap.emplace(arr.at(0), d.at(0) == ' ' ? d.substr(1) : d);
		}
	}

	void ConnectionBase::NotifyAppendResponse(const char* contents, size_t size) {
		ASSERT(mOngoingRequest);

		if (mCompletedRequest) {
			// request probably timed out so discard data
			return;
		}

		SetResponseData(contents, size);
	}

	uint32_t ConnectionBase::GetResponseSize() {
		return mResponseSize;
	}

	std::string_view ConnectionBase::GetHeader(const std::string& key) {
		return mHeaderMap.at(key);
	}


}
