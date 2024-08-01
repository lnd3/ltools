#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// defined windows lean and mean or include winsock before windows header...
// https://stackoverflow.com/questions/1372480/c-redefinition-header-files-winsock2-h
//
// Server example
// https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
// Client example
// https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code
// Best practices
// https://docs.microsoft.com/en-us/windows/win32/winsock/performance-needs-users-and-administrators-2


#include <WS2tcpip.h>
#include <memory>

#pragma comment(lib, "Ws2_32.lib")

namespace l {
namespace x {
namespace net {

	namespace details {
		int xsend(SOCKET s, const char *buf, size_t len, int flags);
		int xrecv(SOCKET s, char *buf, size_t len, int flags);
	}

	class WSAConnection;

	class WSAConnectionListener {
	public:
		WSAConnectionListener(SOCKET listenSocket, unsigned short port);
		~WSAConnectionListener();

		std::unique_ptr<WSAConnection> Accept();
	protected:
		SOCKET mListenSocket = INVALID_SOCKET;
		unsigned short mPort;
	};

	class WSAConnection {
	public:
		WSAConnection(SOCKET clientSocket, const char* host, unsigned short port);
		~WSAConnection();

		int Write(const char *buf, size_t len, int flags);
		int Read(char *buf, size_t len, int flags);

		template<const size_t SIZE>
		int Read(std::stringstream& stream, int flags) {
			char buf[SIZE];
			int read = details::xrecv(mConnectionSocket, (char *)buf, SIZE - 1, flags);
			if (read > 0) {
				buf[read] = 0;
				stream << buf;
			}
			return read;
		}

	protected:
		SOCKET mConnectionSocket = INVALID_SOCKET;
		const char* mHost;
		unsigned short mPort;
	};

	std::unique_ptr<WSAConnectionListener> Listen(unsigned short port);
	std::unique_ptr<WSAConnection> Connect(const char* host, unsigned short port);

}
}
}
