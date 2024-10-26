#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkInterfaceWS.h"
#include "filesystem/File.h"

#include <array>

using namespace l;

TEST(NetworkWebSocket, Setup) {

	auto networkManager = l::network::CreateNetworkManager(1, true);
	auto networkInterfaceWS = l::network::CreateNetworkInterfaceWS(networkManager);

	bool failed = false;
	auto websocketHandler = [&](
		bool success,
		std::string_view queryArguments,
		l::network::WebSocket& ws) {
			TEST_TRUE_NO_RET(success, "");
			TEST_TRUE_NO_RET(ws.IsWebSocket(), "");
			failed = !success;
			LOG(LogInfo) << "Query arguments: '" << queryArguments << "'";
			LOG(LogInfo) << ws.GetResponse().str();

			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	networkInterfaceWS->CreateInterface("Websocket", "wss", "echo.websocket.org");
	networkInterfaceWS->CreateWebSocketTemplate<std::stringstream>("Websocket", "wsstest", "", websocketHandler);
	networkInterfaceWS->Connect("Websocket", "wsstest");

	char buffer[1024];
	int32_t read = 0;
	int32_t readCount = 20;
	do {
		read = networkInterfaceWS->Read("Websocket", "wsstest", &buffer[0], 1024);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
