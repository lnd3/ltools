#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkInterface.h"
#include "filesystem/File.h"
#include "serialization/TrivialData.h"

#include <array>

using namespace l;

TEST(NetworkInterface, Setup) {

	std::stringstream configData;
	if (!l::filesystem::read("tests/telegrambottoken.txt", configData)) {
		return 0;
	}
	auto configMap = l::serialization::ParseTrivialDataMap(configData, "=\n");
	auto& telegramToken = configMap[l::string::string_id("telegram_token")];
	auto& telegramChatId = configMap[l::string::string_id("telegram_chat_id")];

	auto networkManager = l::network::CreateNetworkManager(2, false);
	auto networkInterface = l::network::CreateNetworkInterface(networkManager);

	std::string query = "bot";
	query += telegramToken;
	query += "/sendMessage?";

	bool failed = false;

	auto telegramHandler = [&](
		bool success,
		std::string_view queryArguments,
		l::network::RequestStringStream& request) {
			TEST_TRUE_NO_RET(success, "");

			failed = !success;

			LOG(LogInfo) << "Query arguments: '" << queryArguments << "'";
			LOG(LogInfo) << request.GetResponse().str();
			return success ? l::concurrency::RunnableResult::SUCCESS : l::concurrency::RunnableResult::FAILURE;
		};

	networkInterface->CreateInterface("Telegram", "https", "api.telegram.org");
	networkInterface->CreateRequestTemplate<std::stringstream>("Telegram", "TradeFlowBot1", query, 1, 2000, 5, telegramHandler);

	std::string args;
	std::string message = "NetworkInterface tester app";
	args += "chat_id=" + telegramChatId;
	args += "&text=" + l::string::encode_html(message);
	args += "&parse_mode=HTML";

	networkInterface->SendRequest("Telegram", "TradeFlowBot1", args, 1, 2000, 5, nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(20));

	networkManager->ClearJobs();
	networkManager->Shutdown();

	TEST_FALSE(failed, "");

	return 0;
}
