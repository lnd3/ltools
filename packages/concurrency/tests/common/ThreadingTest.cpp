#include "testing/Test.h"
#include "logging/Log.h"

#include "concurrency/ExecutorService.h"

using namespace l;


TEST(Threading, ExecutorServiceIdling) {
	l::concurrency::ExecutorService executor("executor service tester", 100);
	//executor.gDebugLogging = true;

	executor.startJobs();
	executor.pauseJobs();
	return 0;
}

TEST(Threading, ExecutorServiceStressTest) {

	std::atomic_int32_t completedCount = 0;
	std::atomic_int32_t abortedCount = 0;
	{
		int numJobs = 1000;

		l::concurrency::ExecutorService executor("executor service tester", 100, numJobs);
		//executor.gDebugLogging = true;

		int innerLoops = 100000;

		LOG(LogInfo) << "Running " << numJobs << " jobs each doing " << innerLoops << "x some simple work.";

		for (int i = 0; i < numJobs; i++) {
			bool result = executor.queueJob(std::make_unique<l::concurrency::Worker>(
				"Worker " + std::to_string(i),
				[index = i, loops = innerLoops, &completedCount, &abortedCount](const l::concurrency::RunState& state) {
					//LOG(LogDebug) << "Updating thread " << index;
					for (int j = 0; j < loops; j++) {
						j--;
						j++;
						j++;
						if (state.IsShuttingDown()) {
							abortedCount++;
							//LOG(LogDebug) << "Breaking thread looping for shutdown on thread " << index;
							return l::concurrency::RunnableResult::FAILURE;
						}
					}
					completedCount++;

					return l::concurrency::RunnableResult::SUCCESS;
				}
			));
		}

		executor.startJobs();

		LOG(LogInfo) << "Ran " << completedCount << " jobs";

		executor.pauseJobs();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		executor.startJobs();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));

	}
	LOG(LogInfo) << "Ran " << completedCount << " and aborted " << abortedCount << " jobs";

	auto totalCount = completedCount + abortedCount;

	TEST_EQ(totalCount, 1000, "Count was wrong");

	LOG(LogInfo) << "Ran a total of " << totalCount << " jobs";

	return 0;
}

TEST(Threading, ExecutorServiceShutdown) {
	std::atomic_int32_t completedCount = 0;
	std::atomic_int32_t abortedCount = 0;
	{
		int numJobs = 5000;

		l::concurrency::ExecutorService executor("executor service tester", 100, numJobs);
		//executor.gDebugLogging = true;

		int innerLoops = 10000;

		LOG(LogInfo) << "Running " << numJobs << " jobs each doing " << innerLoops << "x some simple work.";

		for (int i = 0; i < numJobs; i++) {
			bool result = executor.queueJob(std::make_unique<l::concurrency::Worker>(
				"Worker " + std::to_string(i),
				[index = i, loops = innerLoops, &completedCount, &abortedCount](const l::concurrency::RunState& state) {
					//LOG(LogDebug) << "Updating thread " << index;
					for (int j = 0; j < loops; j++) {
						j--;
						j++;
						j++;
						if (state.IsShuttingDown()) {
							abortedCount++;
							//LOG(LogDebug) << "Breaking thread looping for shutdown on thread " << index;
							return l::concurrency::RunnableResult::FAILURE;
						}
					}
					completedCount++;

					return l::concurrency::RunnableResult::SUCCESS;
				}
			));
		}

		executor.startJobs();

		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
	LOG(LogInfo) << "Ran " << completedCount << " and aborted " << abortedCount << " jobs";


	auto totalCount = completedCount + abortedCount;

	TEST_EQ(totalCount, 5000, "Count was wrong");

	LOG(LogInfo) << "Ran a total of " << totalCount << " jobs";

	return 0;
}
