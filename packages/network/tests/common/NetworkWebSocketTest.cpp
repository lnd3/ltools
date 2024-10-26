#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkInterfaceWS.h"
#include "filesystem/File.h"

#include <array>

using namespace l;

TEST(NetworkWebSocket, Setup) {

	auto networkManager = l::network::CreateNetworkManager(2, false);
	auto networkInterfaceWS = l::network::CreateNetworkInterfaceWS(networkManager);

	bool failed = false;
	auto websocketHandler = [&](
		bool success,
		std::string_view queryArguments,
		l::network::RequestStringStream& request) {
			TEST_TRUE_NO_RET(success, "");
			TEST_TRUE_NO_RET(request.IsWebSocket(), "");
			failed = !success;
			LOG(LogInfo) << "Query arguments: '" << queryArguments << "'";
			LOG(LogInfo) << request.GetResponse().str();
			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	networkInterfaceWS->CreateInterface("Websocket", "wss", "echo.websocket.org");
	networkInterfaceWS->CreateWebSocketTemplate<std::stringstream>("Websocket", "wsstest", "", 1, 2000, 15, websocketHandler);
	networkInterfaceWS->Connect("Websocket", "wsstest");
	networkInterfaceWS->Send("Websocket", "wsstest", "");

	std::this_thread::sleep_for(std::chrono::milliseconds(20));

	networkInterfaceWS->Shutdown();
	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

	return 0;
}
