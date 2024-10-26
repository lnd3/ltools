#include "network/NetworkConnection.h"

namespace l::network {

	int CurlClientCloseSocket(void* userdata, curl_socket_t item) {
		auto request = reinterpret_cast<RequestBase*>(userdata);
		if (request != nullptr) {
			request->NotifyClose();
		}
		LOG(LogDebug) << "Socket close: " << item;
		return 0;
	}

	size_t CurlClientWriteHeader(char* contents, size_t size, size_t nitems, void* userdata) {
		auto request = reinterpret_cast<RequestBase*>(userdata);
		if (request != nullptr) {
			request->NotifyAppendHeader(contents, size * nitems);
		}
		return size * nitems;
	}

	size_t CurlClientWriteCallback(char* contents, size_t size, size_t nmemb, void* userdata) {
		auto request = reinterpret_cast<RequestBase*>(userdata);
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
}
