#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkInterfaceWS.h"
#include "filesystem/File.h"
#include "jsonxx/jsonxx.h"

#include <array>
#include <iostream>

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
	networkInterfaceWS->CreateWebSocket("Websocket", "wsstest", "", websocketHandler);
	networkInterfaceWS->Connect("Websocket", "wsstest");

	char buffer[1024];
	int32_t read = 0;
	int32_t readCount = 20;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		read = networkInterfaceWS->Read("Websocket", "wsstest", &buffer[0], 1024);
	} while (read <= 0 && readCount-- >= 0);

	TEST_TRUE(read > 0, "");

	LOG(LogInfo) << std::string_view(buffer, read);

	networkInterfaceWS->Disconnect("wsstest");

	networkInterfaceWS->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

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
			LOG(LogInfo) << "Success: " << success;

			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	networkInterfaceWS->CreateInterface("Binance", "wss", "testnet.binance.vision");
	//networkInterfaceWS->CreateInterface("Binance", "wss", "stream.binance.com", 443);
	networkInterfaceWS->CreateWebSocket("Binance", "binance", "ws", websocketHandler);
	networkInterfaceWS->Connect("Binance", "binance");

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
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		read = networkInterfaceWS->Read("Binance", "binance", &buffer[0], 1024);
		if (read > 0) {
			LOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);

	networkInterfaceWS->Write("Binance", "binance", subscribeStream.data(), subscribeStream.size());


	readCount = 60;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		read = networkInterfaceWS->Read("Binance", "binance", &buffer[0], 1024);
		if (read > 0) {
			LOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);


	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	networkInterfaceWS->Disconnect("binance");

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
			LOG(LogInfo) << "Success: " << success;

			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	// wss://testnet.binance.vision/ws-api/v3
	networkInterfaceWS->CreateInterface("Binance", "wss", "testnet.binance.vision");
	//networkInterfaceWS->CreateInterface("Binance", "wss", "stream.binance.com", 443);
	networkInterfaceWS->CreateWebSocket("Binance", "binance", "ws-api/v3", websocketHandler);
	networkInterfaceWS->Connect("Binance", "binance");

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
		read = networkInterfaceWS->Read("Binance", "binance", &buffer[0], 1024);
		if (read > 0) {
			LOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);

	networkInterfaceWS->Write("Binance", "binance", requestKlines.data(), requestKlines.size());


	readCount = 60;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		read = networkInterfaceWS->Read("Binance", "binance", &buffer[0], 1024);
		if (read > 0) {
			LOG(LogInfo) << std::string_view(buffer, read);
		}
	} while (readCount-- >= 0);


	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	networkInterfaceWS->Disconnect("binance");

	networkInterfaceWS->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

	return 0;
}
