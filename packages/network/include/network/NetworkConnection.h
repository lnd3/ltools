#pragma once

#include <math.h>
#include <string>
#include <sstream>
#include <functional>
#include <vector>
#include <atomic>

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

	class ConnectionBase {
	public:
		ConnectionBase() :
			mCurl(nullptr),
			mOngoingRequest(false),
			mResponseSize(0),
			mDefaultResponseSize(4000),
			mTimeout(0),
			mCompletedRequest(true),
			mStarted(0),
			mSuccess(false)
		{}
		ConnectionBase(
			std::string_view name,
			std::string_view query,
			int32_t defaultResponseSize,
			int32_t timeout = 0
		) :
			mCurl(nullptr),
			mName(name),
			mRequestQuery(query),
			mOngoingRequest(false),
			mResponseSize(0),
			mDefaultResponseSize(defaultResponseSize),
			mTimeout(timeout),
			mCompletedRequest(true),
			mStarted(0),
			mSuccess(false)
		{}

		virtual ~ConnectionBase() {
			if (mCurl) {
				curl_easy_cleanup(mCurl);
			}
		}

		std::string_view GetRequestName();
		bool TryReservingRequest();
		l::concurrency::RunnableResult SendAndUnReserveRequest(
			CURLM* multiHandle,
			const l::concurrency::RunState& state,
			const std::string& queryArguments,
			const std::string& query,
			const int32_t expectedResponseSize = 0,
			const int32_t timeOut = -1,
			std::function<void(bool, std::string_view)> cb = nullptr);
		bool IsHandle(CURL* handle);
		bool IsWebSocket();
		bool HasExpired();

		bool WSWrite(char* buffer, size_t size);
		int32_t WSRead(char* buffer, size_t size);
		void WSClose();

		const curl_ws_frame* GetWebSocketMeta();

		void NotifyClose();
		void NotifyCompleteRequest(bool success);
		void NotifyAppendHeader(const char* contents, size_t size);
		void NotifyAppendResponse(const char* contents, size_t size);

		uint32_t GetResponseSize();

		virtual void SetResponseSize(int32_t expectedResponseSize) = 0;
		virtual void SetResponseData(const char* contents, size_t size) = 0;
		virtual l::concurrency::RunnableResult CallHandler() = 0;

	protected:
		CURL* mCurl = nullptr;

		std::string mName;
		std::string mRequestQuery;
		std::string mRequestQueryArgs;
		std::atomic_bool mOngoingRequest;
		int32_t mResponseSize;
		int32_t mDefaultResponseSize;
		std::atomic_int32_t mTimeout;

		std::atomic_bool mCompletedRequest;

		std::vector<std::string> mHeaders;
		std::unordered_map<std::string, std::string> mHeaderMap;
		int64_t mStarted;
		std::atomic_bool mSuccess;
		bool mIsWebSocket = false;
	};

	template<class T>
	class Request : public ConnectionBase {
	public:
		Request(
			std::string_view name, 
			std::string_view query, 
			int32_t defaultResponseSize,
			std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, Request<T>&)> handler,
			int32_t timeout = 0 // no timeout
		) : ConnectionBase(name, query, defaultResponseSize, timeout),
			mHandler(handler)
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
			this->mResponseSize = other.mResponseSize;
			this->mDefaultResponseSize = other.mDefaultResponseSize;
			this->mTimeout.store(other.mTimeout);
			this->mCompletedRequest = other.mCompletedRequest;
			this->mRequestQueryArgs = other.mRequestQueryArgs;
			this->mStarted = other.mStarted;
			this->mSuccess = other.mSuccess;
			return *this;
		}
		Request& operator=(const Request& other) noexcept {
			this->mName = other.mName;
			this->mRequestQuery = other.mRequestQuery;
			this->mHandler = other.mHandler;
			this->mResponse.str("");
			this->mResponse << other.mResponse.rdbuf();
			this->mOngoingRequest = false;
			this->mResponseSize = other.mResponseSize;
			this->mDefaultResponseSize = other.mDefaultResponseSize;
			this->mTimeout.store(other.mTimeout);
			this->mCompletedRequest = other.mCompletedRequest;
			this->mRequestQueryArgs = other.mRequestQueryArgs;
			this->mStarted = other.mStarted;
			this->mSuccess = other.mSuccess;
			return *this;
		}
		Request(Request&& other) noexcept {
			*this = std::move(other);
		}
		Request(const Request& other) noexcept {
			*this = other;
		}

		void SetResponseSize(int32_t expectedResponseSize) {
			if constexpr (std::is_same_v<std::vector<unsigned char>, T>) {
				if (expectedResponseSize > 0) {
					mDefaultResponseSize = expectedResponseSize;
				}
				mResponse.resize(mDefaultResponseSize);
			}
			else if constexpr (std::is_same_v<std::stringstream, T>) {
				mResponse.str("");
			}
		}

		l::concurrency::RunnableResult CallHandler() {
			if (mHandler) {
				return mHandler(mSuccess, mRequestQueryArgs, *this);
			}
			return l::concurrency::RunnableResult::CANCELLED;
		}

		void SetResponseData(const char* contents, size_t size) {
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

	protected:
		std::function<l::concurrency::RunnableResult(bool success, std::string_view queryArguments, Request<T>&)> mHandler;
		T mResponse;
	};

	using RequestStringStream = Request<std::stringstream>;
	using RequestBinaryStream = Request<std::vector<unsigned char>>;
	using WebSocket = Request<std::stringstream>;

}
