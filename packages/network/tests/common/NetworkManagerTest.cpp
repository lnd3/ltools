#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkManager.h"
#include "filesystem/File.h"

using namespace l;

TEST(NetworkManager, Setup) {

	auto networkManager = l::network::CreateNetworkManager(2, false);

	std::filesystem::path filepath = "tests/testdata.txt";
	std::filesystem::remove(filepath);
	l::filesystem::File file(filepath);

	auto request1 = std::make_unique<l::network::RequestStringStream>("requestName", "https://httpbin.org/anything", 25000,
		[&](bool success, std::string_view queryArguments, l::network::RequestStringStream& request) {
			TEST_TRUE_NO_RET(success, "");

			LOG(LogInfo) << "Query arguments: '" << queryArguments << "'";
			file.modeWriteTrunc();
			if (file.open()) {
				file.write(request.GetResponse());
			}
			file.close();
			return l::concurrency::RunnableResult::SUCCESS;
		}
	);
	auto request2 = std::make_unique<l::network::RequestStringStream>("requestName", "https://httpbin.org/anything", 25000,
		[&](bool success, std::string_view queryArguments, l::network::RequestStringStream& request) {
			TEST_TRUE_NO_RET(success, "");

			LOG(LogInfo) << "Query arguments: '" << queryArguments << "'";
			LOG(LogInfo) << request.GetResponse().str();
			return l::concurrency::RunnableResult::SUCCESS;
		}
	);


	networkManager->CreateRequestTemplate(std::move(request1));
	networkManager->CreateRequestTemplate(std::move(request2));
	networkManager->PostQuery("requestName", "user defined query id");
	networkManager->PostQuery("requestName", "custom queries on predefined requests", 3, "https://httpbin.org/anything");

	TEST_TRUE(networkManager->TotalRequestCount() == 2, "");

	networkManager->Shutdown();

	TEST_TRUE(std::filesystem::exists(filepath), "File was not created");

	return 0;
}
