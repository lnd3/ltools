#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkInterface.h"
#include "filesystem/File.h"

#include <array>

using namespace l;

TEST(NetworkInterface, Setup) {

	std::stringstream telegramToken;
	if (!l::filesystem::read("tests/telegrambottoken.txt", telegramToken)) {
		return 0;
	}

	auto networkManager = l::network::CreateNetworkManager(2, false);
	auto networkInterface = l::network::CreateNetworkInterface(networkManager);

	std::string query = "bot";
	query += telegramToken.str();
	query += "/sendMessage?";

	auto telegramHandler = [&](
		bool success,
		std::string_view queryArguments,
		l::network::RequestStringStream& request) {
			TEST_TRUE_NO_RET(success, "");

			LOG(LogInfo) << "Query arguments: '" << queryArguments << "'";
			LOG(LogInfo) << request.GetResponse().str();
			return l::concurrency::RunnableResult::SUCCESS;
		};

	networkInterface->CreateInterface("Telegram", "https", "api.telegram.org");
	networkInterface->CreateRequestTemplate<std::stringstream>("Telegram", "TradeFlowBot1", query, 1, 2000, 5, telegramHandler);

	std::string chatId = "6640331275"; // TradeFlowGroup
	std::string args;
	std::string message = "NetworkInterface tester app";
	args += "chat_id=" + chatId;
	args += "&text=" + l::string::encode_html(message);
	args += "&parse_mode=HTML";

	networkInterface->SendRequest("Telegram", "TradeFlowBot1", args, 1, 2000, 5, nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(20));

	networkManager->ClearJobs();
	networkManager->Shutdown();

	return 0;
}
