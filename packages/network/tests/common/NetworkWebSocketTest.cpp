#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkInterfaceWS.h"
#include "filesystem/File.h"
#include "jsonxx/jsonxx.h"

#include <array>
#include <iostream>
#include <thread>

#if defined(__linux__)
#include <filesystem>
#include <unistd.h>
#endif

using namespace l;

TEST(NetworkWebSocket, Setup) {

	return 0;

	auto networkManager = l::network::CreateNetworkManager(1, true);
	auto networkInterfaceWS = l::network::CreateNetworkInterfaceWS(networkManager);

	bool failed = false;
	auto websocketHandler = [&](
		bool success,
		std::string_view,
		l::network::WebSocket& ws) {
			TEST_TRUE_NO_RET(success, "");
			TEST_TRUE_NO_RET(ws.IsWebSocket(), "");
			failed = !success;
			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	networkInterfaceWS->CreateInterface("Websocket", "wss", "echo.websocket.org");
	networkInterfaceWS->CreateWebSocket("Websocket", "", websocketHandler);
	networkInterfaceWS->Connect("Websocket");

	char buffer[1024];
	int32_t read = 0;
	int32_t readCount = 20;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		read = networkInterfaceWS->Read("Websocket", &buffer[0], 1024);
	} while (read <= 0 && readCount-- >= 0);

	TEST_TRUE(read > 0, "");

	LLOG(LogInfo) << std::string_view(buffer, read);

	networkInterfaceWS->Disconnect("Websocket");

	networkInterfaceWS->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

	return 0;
}

namespace {
	// open sockets of this process (Linux); -1 elsewhere
	int32_t CountOpenSockets() {
#if defined(__linux__)
		int32_t n = 0;
		for (const auto& e : std::filesystem::directory_iterator("/proc/self/fd")) {
			char buf[64] = {};
			auto len = readlink(e.path().c_str(), buf, sizeof(buf) - 1);
			if (len > 0 && std::string_view(buf, static_cast<size_t>(len)).substr(0, 7) == "socket:") n++;
		}
		return n;
#else
		return -1;
#endif
	}
}

// Offline: a websocket whose connect fails (refused, or an unresolvable host) must never report connected, and the
// failed attempts must not leak sockets. Both regressed on 2026-11-03 (a failed connect set the handshake flag and sat
// "connected"; the close-socket callback never closed the socket).
TEST(NetworkWebSocket, FailedConnectIsNotConnected) {
	auto networkManager = l::network::CreateNetworkManager(1, false);
	auto ws = l::network::CreateNetworkInterfaceWS(networkManager);

	const int32_t socketsBefore = CountOpenSockets();
	bool everConnected = false;
	struct Target { const char* name; const char* host; uint16_t port; };
	const Target targets[] = { {"Refused", "127.0.0.1", 1}, {"Unresolved", "no-such-host.invalid", 443} };
	for (const auto& t : targets) {
		for (int32_t attempt = 0; attempt < 5; attempt++) {
			ws->CreateInterface(t.name, "wss", t.host, t.port);
			ws->CreateWebSocket(t.name, "ws");
			ws->Connect(t.name, 0, 0, -1, nullptr);
			for (int32_t i = 0; i < 20; i++) {   // 1 s
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				everConnected |= ws->IsConnected(t.name);
			}
		}
	}
	const int32_t socketsAfter = CountOpenSockets();
	LLOG(LogInfo) << "sockets before " << socketsBefore << ", after 10 failed connects " << socketsAfter;

	ws->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(everConnected, "a failed connect reported connected");
	if (socketsBefore >= 0) {
		TEST_TRUE(socketsAfter <= socketsBefore + 1, "failed connects leaked sockets");
	}
	return 0;
}

TEST(NetworkWebSocket, BinanceKlines) {

	return 0;

	auto networkManager = l::network::CreateNetworkManager(1, false);
	auto networkInterfaceWS = l::network::CreateNetworkInterfaceWS(networkManager);

	bool failed = false;
	auto websocketHandler = [&](
		bool success,
		std::string_view,
		l::network::WebSocket& ws) {
			failed = !success;
			LLOG(LogInfo) << "Success: " << success;

			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	networkInterfaceWS->CreateInterface("Binance", "wss", "testnet.binance.vision");
	//networkInterfaceWS->CreateInterface("Binance", "wss", "stream.binance.com", 443);
	networkInterfaceWS->CreateWebSocket("Binance", "ws", websocketHandler);
	networkInterfaceWS->Connect("Binance");

	char buffer[1024];
	int32_t write = 0;

	std::string subscribeStream;
	subscribeStream += "{\n";
	subscribeStream += "\"method\": \"SUBSCRIBE\",\n";
	subscribeStream += "\"params\" : [";
	//subscribeStream += "\"btcusdt@kline_1m\",";
	//subscribeStream += "\"dogeusdt@kline_1m\",";
	//subscribeStream += "\"solusdt@kline_1m\"";
	subscribeStream += "\"solusdt@kline_1m\"";
	subscribeStream += "],\n";
	subscribeStream += "\"id\" : 1\n";
	subscribeStream += "}\n";

	int32_t read = 0;
	int32_t readCount = 3;

	networkInterfaceWS->QueueWrite("Binance", subscribeStream.data(), subscribeStream.size());

	readCount = 60;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		read = networkInterfaceWS->Read("Binance", &buffer[0], 1024);
		if (read > 0) {
			LLOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);


	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	networkInterfaceWS->Disconnect("Binance");

	networkInterfaceWS->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

	return 0;
}

TEST(NetworkWebSocket, BinanceUIKlines) {

	return 0;

	auto networkManager = l::network::CreateNetworkManager(1, false);
	auto networkInterfaceWS = l::network::CreateNetworkInterfaceWS(networkManager);

	bool failed = false;
	auto websocketHandler = [&](
		bool success,
		std::string_view queryArguments,
		l::network::WebSocket& ws) {
			failed = !success;
			LLOG(LogInfo) << "Success: " << success;

			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	// wss://testnet.binance.vision/ws-api/v3
	networkInterfaceWS->CreateInterface("Binance", "wss", "testnet.binance.vision");
	//networkInterfaceWS->CreateInterface("Binance", "wss", "stream.binance.com", 443);
	networkInterfaceWS->CreateWebSocket("Binance", "ws-api/v3", websocketHandler);
	networkInterfaceWS->Connect("Binance");

	char buffer[1024];
	int32_t write = 0;
	int64_t time = l::string::get_unix_epoch() - 60 * 15;
	time *= 1000;

	std::string requestKlines;
	requestKlines += "{\n";
	requestKlines += "\"id\" : 3,\n";
	requestKlines += "\"method\": \"uiKlines\",\n";
	requestKlines += "\"params\" : {";

	requestKlines += "\"symbol\" : \"BTCUSDT\",";
	requestKlines += "\"interval\" : \"5m\",";
	requestKlines += "\"startTime\" : " + std::to_string(time) + ", ";
	requestKlines += "\"limit\" : \"500\"";

	requestKlines += "}\n";
	requestKlines += "}\n";


	int32_t read = 0;
	int32_t readCount = 3;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		read = networkInterfaceWS->Read("Binance", &buffer[0], 1024);
		if (read > 0) {
			LLOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);

	networkInterfaceWS->Write("Binance", requestKlines.data(), requestKlines.size());


	readCount = 60;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		read = networkInterfaceWS->Read("Binance", &buffer[0], 1024);
		if (read > 0) {
			LLOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);


	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	networkInterfaceWS->Disconnect("Binance");

	networkInterfaceWS->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

	return 0;
}
