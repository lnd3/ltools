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

namespace l::network {

	int CurlClientCloseSocket(void* userdata, curl_socket_t item);
	size_t CurlClientWriteHeader(char* contents, size_t size, size_t nitems, void* userdata);
	size_t CurlClientWriteCallback(char* contents, size_t size, size_t nmemb, void* userdata);
	size_t CurlClientProgressCallback(void* clientp, double dltotal, double dlnow, double, double);

	struct Progress {
		const l::concurrency::RunState* state;
		size_t size;
	};

	class RequestBase {
	public:
		RequestBase() = default;
		virtual ~RequestBase() = default;
		virtual std::string_view GetRequestName() = 0;
		virtual bool TryReservingRequest() = 0;
		virtual l::concurrency::RunnableResult SendAndUnReserveRequest(
			CURLM* multiHandle,
			const l::concurrency::RunState& state,
			const std::string& queryArguments,
			const std::string& query,
			const int32_t expectedResponseSize = 0,
			const int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr) = 0;
		virtual bool IsHandle(CURL* handle) = 0;
		virtual bool IsWebSocket() = 0;
		virtual bool HasExpired() = 0;

		virtual bool WSWrite(char* buffer, size_t size) = 0;
		virtual size_t WSRead(char* buffer, size_t size) = 0;
		virtual void WSClose() = 0;

		virtual const curl_ws_frame* GetWebSocketMeta() = 0;

		virtual void NotifyClose() = 0;
		virtual void NotifyCompleteRequest(bool success) = 0;
		virtual void NotifyAppendHeader(const char* contents, size_t size) = 0;
		virtual void NotifyAppendResponse(const char* contents, size_t size) = 0;
	};

	template<class T>
	class Request : public RequestBase {
	public:
		Request(
			std::string_view name, 
			std::string_view query, 
			int32_t defaultResponseSize,
			std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, Request<T>&)> handler,
			int32_t timeout = 0 // no timeout
		) :
			mCurl(nullptr),
			mName(name),
			mRequestQuery(query),
			mHandler(handler),
			mOngoingRequest(false),
			mResponseSize(0),
			mDefaultResponseSize(defaultResponseSize),
			mTimeout(timeout),
			mCompletedRequest(true),
			mStarted(0),
			mSuccess(false)
		{
			if constexpr (std::is_same_v<std::vector<unsigned char>, T>) {
				mResponse.reserve(mDefaultResponseSize);
			}
		}
		Request& operator=(Request&& other) noexcept {
			if (!this->mCurl) {
				this->mCurl = other.mCurl;
				other.mCurl = nullptr;
			}

			this->mName = std::move(other.mName);
			this->mRequestQuery = std::move(other.mRequestQuery);
			this->mHandler = std::move(other.mHandler);
			this->mResponse = std::move(other.mResponse);
			this->mOngoingRequest = false;
			this->mResponseSize = other->mResponseSize;
			this->mDefaultResponseSize = other->mDefaultResponseSize;
			this->mTimeout.store(other->mTimeout);
			this->mCompletedRequest = other->mCompletedRequest;
			this->mRequestQueryArgs = other->mRequestQueryArgs;
			this->mStarted = other->mStarted;
			this->mSuccess = other->mSuccess;
			return *this;
		}
		Request& operator=(const Request& other) noexcept {
			this->mName = other.mName;
			this->mRequestQuery = other.mRequestQuery;
			this->mHandler = other.mHandler;
			this->mResponse.str("");
			this->mResponse << other.mResponse.rdbuf();
			this->mOngoingRequest = false;
			this->mResponseSize = other->mResponseSize;
			this->mDefaultResponseSize = other->mDefaultResponseSize;
			this->mTimeout.store(other->mTimeout);
			this->mCompletedRequest = other->mCompletedRequest;
			this->mRequestQueryArgs = other->mRequestQueryArgs;
			this->mStarted = other->mStarted;
			this->mSuccess = other->mSuccess;
			return *this;
		}
		Request(Request&& other) noexcept {
			*this = std::move(other);
		}
		Request(const Request& other) noexcept {
			*this = other;
		}

		~Request() {
			if (mCurl) {
				curl_easy_cleanup(mCurl);
			}
		}

		std::string_view GetRequestName() override {
			return mName;
		}

		bool TryReservingRequest() override {
			bool available = false;
			if (mCompletedRequest && mOngoingRequest.compare_exchange_strong(available, true)) {
				mCompletedRequest = false;
				mStarted = l::string::get_unix_epoch_ms();
				return true;
			}
			return false;
		}

		l::concurrency::RunnableResult SendAndUnReserveRequest(
			CURLM* multiHandle,
			const l::concurrency::RunState& state, 
			const std::string& queryArguments, 
			const std::string& query, 
			int32_t expectedResponseSize = 0,
			int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr
		) override {
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

			if constexpr (std::is_same_v<std::vector<unsigned char>, T>) {
				if (expectedResponseSize > 0) {
					mDefaultResponseSize = expectedResponseSize;
				}
				mResponse.resize(mDefaultResponseSize);
			}
			else if constexpr (std::is_same_v<std::stringstream, T>) {
				mResponse.str("");
			}

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
				auto curlCode = curl_easy_perform(mCurl);
				//curl_easy_header()
				if (curlCode != CURLE_OK) {
					LOG(LogError) << "Curl failure  " << std::to_string(curlCode) << ": " << mRequestQueryArgs;
					mSuccess = false;
				}
				NotifyCompleteRequest(mSuccess);
			}

			l::concurrency::RunnableResult result = mSuccess ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
			if (mHandler) {
				result = mHandler(mSuccess, mRequestQueryArgs, *this);
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

		void NotifyCompleteRequest(bool success) {
			bool completed = false;
			if (mOngoingRequest && mCompletedRequest.compare_exchange_strong(completed, true)) {
				mSuccess = success;
				return;
			}
		}

		bool IsHandle(CURL* handle) {
			if (mCurl == handle) {
				return true;
			}
			return false;
		}

		bool HasExpired() {
			if (mOngoingRequest && mTimeout > 0) {
				auto timeWaitingMs = static_cast<int32_t>(l::string::get_unix_epoch_ms() - mStarted);
				auto expired = timeWaitingMs > mTimeout * 1000;
				return expired;
			}
			return false;
		}

		void NotifyClose() {

		}

		bool IsWebSocket() {
			return mIsWebSocket;
		}

		const curl_ws_frame* GetWebSocketMeta() {
			const struct curl_ws_frame* m = curl_ws_meta(mCurl);
			return m;
		}

		virtual bool WSWrite(char* buffer, size_t size) {
			size_t sentBytes = 0;
			auto res = curl_ws_send(mCurl, buffer, size, &sentBytes, 0, CURLWS_TEXT);
			ASSERT(sentBytes == size);
			return res == CURLE_OK;
		}

		virtual size_t WSRead(char* buffer, size_t size) {
			const struct curl_ws_frame* meta;
			size_t readBytes = 0;
			auto res = curl_ws_recv(mCurl, buffer, size, &readBytes, &meta);
			return res == CURLE_OK ? readBytes : -1;
		}

		virtual void WSClose() {

		}

		void NotifyAppendHeader(const char* contents, size_t size) {
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
			else if (arr.size() >= 2){
				auto& d = arr.at(1);
				mHeaderMap.emplace(arr.at(0), d.at(0) == ' ' ? d.substr(1) : d);
			}
		}

		void NotifyAppendResponse(const char* contents, size_t size) {
			ASSERT(mOngoingRequest);

			if (mCompletedRequest) {
				// request probably timed out so discard data
				return;
			}

			if constexpr (std::is_base_of_v<std::vector<unsigned char>, T>) {
				auto totalSize = mResponseSize + size;
				if (totalSize > mDefaultResponseSize) {
					mDefaultResponseSize *= 2;
				}
				if (mResponse.size() < mDefaultResponseSize) {
					mResponse.resize(mDefaultResponseSize);
				}
				char* base = reinterpret_cast<char*>(mResponse.data() + mResponseSize);
				memcpy(base, contents, size);
			}
			else if constexpr (std::is_base_of_v<std::stringstream, T>) {
				mResponse << std::string_view((char*)contents, size);
			}
			mResponseSize += static_cast<int32_t>(size);
		}

		T& GetResponse() {
			return mResponse;
		}

		uint32_t GetResponseSize() {
			return mResponseSize;
		}


	protected:
		CURL* mCurl = nullptr;

		std::string mName;
		std::string mRequestQuery;
		std::string mRequestQueryArgs;
		std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, Request<T>&)> mHandler;
		std::atomic_bool mOngoingRequest;
		int32_t mResponseSize;
		int32_t mDefaultResponseSize;
		std::atomic_int32_t mTimeout;

		std::atomic_bool mCompletedRequest;

		std::vector<std::string> mHeaders;
		std::unordered_map<std::string, std::string> mHeaderMap;
		T mResponse;
		int64_t mStarted;
		std::atomic_bool mSuccess;
		bool mIsWebSocket = false;
	};

	using RequestStringStream = Request<std::stringstream>;
	using RequestBinaryStream = Request<std::vector<unsigned char>>;

}
