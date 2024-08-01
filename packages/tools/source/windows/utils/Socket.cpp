#include "Socket.h"

#include "logging/Log.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

namespace l {
	namespace x {
		namespace net {
			namespace details {
				int xsend(SOCKET s, const char *buf, size_t len, int flags) {
					int result = send(s, (const char *)buf, (int)len, flags);
					if (result == SOCKET_ERROR) {
						LOG(LogWarning) << "Failed to write to socket " << s << ", error: " << WSAGetLastError();
						return result;
					}
					if (result == 0) {
						LOG(LogWarning) << "Socket " << s << " closed gracefully";
						return result;
					}
					return result;
				}

				int xrecv(SOCKET s, char *buf, size_t len, int flags) {
					int result = recv(s, (char *)buf, (int)len, flags);
					if (result == SOCKET_ERROR) {
						LOG(LogWarning) << "Failed to read from socket " << s;
						return result;
					}
					if (result == 0) {
						LOG(LogWarning) << "Socket " << s << " closed gracefully";
						return result;
					}
					return result;
				}

				bool xclose(SOCKET clientSocket) {
					int result = shutdown(clientSocket, SD_SEND);
					if (result == SOCKET_ERROR) {
						LOG(LogError) << "Shutdown failed with error: " << WSAGetLastError();
						closesocket(clientSocket);
						return false;
					}
					closesocket(clientSocket);
					return true;
				}
			}

			void WSAInitialize() {
				static bool initialized = false;
				if (initialized) {
					return;
				}

				WORD version = MAKEWORD(2, 2);
				WSADATA WSAData = { 0 };

				auto iResult = WSAStartup(version, &WSAData);
				if (iResult != 0) {
					LOG(LogError) << "WSAStartup failed with error: " << iResult;
				}
				if (LOBYTE(WSAData.wVersion) == LOBYTE(version) && HIBYTE(WSAData.wVersion) == HIBYTE(version)) {
					initialized = true;
				}
				else {
					WSACleanup();
					return;
				}
			}

			WSAConnectionListener::WSAConnectionListener(SOCKET listenSocket, unsigned short port)
				: mListenSocket(listenSocket), mPort(port) {
			}

			WSAConnectionListener::~WSAConnectionListener() {
				closesocket(mListenSocket);
			}

			std::unique_ptr<WSAConnection> WSAConnectionListener::Accept() {
				SOCKADDR_IN addr;
				int addrlen = sizeof(addr);
				SOCKET s = accept(mListenSocket, (SOCKADDR*)&addr, &addrlen);
				const char *ip = inet_ntoa(addr.sin_addr);
				if (s == INVALID_SOCKET) {
					LOG(LogError) << "Failed connection from " << ip << ":" << addr.sin_port << ", error: " << WSAGetLastError();
					return nullptr;
				}
				LOG(LogInfo) << "Accept connection from " << ip << ":" << addr.sin_port;
				return std::make_unique<WSAConnection>(s, ip, addr.sin_port);
			}

			WSAConnection::WSAConnection(SOCKET socket, const char* host, unsigned short port)
				: mConnectionSocket(socket), mHost(host), mPort(port) {
			}

			WSAConnection::~WSAConnection() {
				details::xclose(mConnectionSocket);
			}

			int WSAConnection::Write(const char *buf, size_t len, int flags)
			{
				return details::xsend(mConnectionSocket, (const char *)buf, (int)len, flags);
			}

			int WSAConnection::Read(char *buf, size_t len, int flags)
			{
				return details::xrecv(mConnectionSocket, (char *)buf, (int)len, flags);
			}

			std::unique_ptr<WSAConnectionListener> Listen(unsigned short port) {
				
				WSAInitialize();

				struct addrinfo *result = NULL;
				struct addrinfo hints;

				ZeroMemory(&hints, sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				hints.ai_flags = AI_PASSIVE;

				// Resolve the server address and port
				int iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
				if (iResult != 0) {
					LOG(LogError) << "getaddrinfo failed with error: " << iResult;
					return nullptr;
				}

				// Create a SOCKET for connecting to server
				SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
				if (listenSocket == INVALID_SOCKET) {
					LOG(LogError) << "socket failed with error: " << WSAGetLastError();
					freeaddrinfo(result);
					return nullptr;
				}

				// Setup the TCP listening socket
				iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
				if (iResult == SOCKET_ERROR) {
					LOG(LogError) << "bind failed with error: " << WSAGetLastError();
					freeaddrinfo(result);
					closesocket(listenSocket);
					return nullptr;
				}

				freeaddrinfo(result);

				iResult = listen(listenSocket, SOMAXCONN);
				if (iResult == SOCKET_ERROR) {
					LOG(LogError) << "listen failed with error: " << WSAGetLastError();
					closesocket(listenSocket);
					return nullptr;
				}

				return std::make_unique<WSAConnectionListener>(listenSocket, port);
			}

			std::unique_ptr<WSAConnection> Connect(const char* host, unsigned short port) {

				WSAInitialize();

				SOCKET connectSocket = INVALID_SOCKET;
				struct addrinfo *result = NULL, *ptr = NULL, hints;
				int iResult;

				ZeroMemory(&hints, sizeof(hints));
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;

				// Resolve the server address and port
				iResult = getaddrinfo(host, std::to_string(port).c_str(), &hints, &result);
				if (iResult != 0) {
					LOG(LogError) << "getaddrinfo failed with error: " << iResult;
					return nullptr;
				}

				// Attempt to connect to an address until one succeeds
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

					// Create a SOCKET for connecting to server
					connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
					if (connectSocket == INVALID_SOCKET) {
						LOG(LogError) << "socket failed with error: " << WSAGetLastError();
						return nullptr;
					}

					// Connect to server.
					iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
					if (iResult == SOCKET_ERROR) {
						closesocket(connectSocket);
						connectSocket = INVALID_SOCKET;
						continue;
					}
					break;
				}

				freeaddrinfo(result);

				if (connectSocket == INVALID_SOCKET) {
					LOG(LogError) << "Unable to connect to server!";
					return nullptr;
				}

				return std::make_unique<WSAConnection>(connectSocket, host, port);
			}

		}
	}
}
