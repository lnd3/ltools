#include "concurrency/ExecutorService.h"

#include <math.h>
#include <cmath>
#include <stdlib.h>

#include "logging/String.h"

namespace l::concurrency {

	bool RunState::IsShuttingDown() const {
		return mDestructing;
	}

	bool RunState::IsPaused() const {
		return !mRunning;
	}

	bool RunState::HasRunningJobs() const {
		return mNumRunningJobs > 0;
	}

	bool RunState::HasRunningJobThreads() const {
		return mNumRunningJobThreads > 0;
	}

	bool RunState::IsShutdown() const {
		return mDestructing && mNumRunningJobs == 0 && mNumRunningJobThreads == 0;
	}

	std::string Runnable::Name() const {
		return mName;
	};

	bool Runnable::CanRun(int64_t time) {
		return time >= mNextTry;
	}

	void Runnable::Reschedule() {
		auto time = l::string::get_unix_epoch_ms();
		mNextTry = time + static_cast<int64_t>(500 + 500 * (rand() / (float)RAND_MAX)); // retry within 1 second
	}

	void Runnable::Backoff() {
		mTries++;
		auto time = l::string::get_unix_epoch_ms();
		mNextTry = time + static_cast<int64_t>(round(powf(static_cast<float>(mTries), 2.5f))); // 1 sec, 3 sec, 5 sec, 8 sec, 11 sec etc
	}

	bool Runnable::Failed() {
		return mTries >= mMaxTries;
	}

	int32_t Runnable::NumTries() {
		return mTries;
	}

	RunnableResult Runnable::run(const RunState&) {
		LOG(LogInfo) << "Default run implementation";
		return RunnableResult::SUCCESS;
	}
	
	RunnableResult Worker::run(const RunState& state) {
		return mWork(state);
	}

	int32_t ExecutorService::numJobs() {
		std::lock_guard<std::mutex> lock(mRunnablesMutex);
		return static_cast<int32_t>(mRunnables.size());
	}

	int32_t ExecutorService::numTotalJobs() {
		return mNumTotalRequests;
	}

	int32_t ExecutorService::numCompletedJobs() {
		return mNumCompletedJobs;
	}

	bool ExecutorService::isShuttingDown() {
		return mRunState.IsShuttingDown();
	}

	void ExecutorService::shutdown() {
		if (mRunState.mDestructing) {
			return;
		}
		if (gDebugLogging) LOG(LogDebug) << "Executor service shutdown is imminent";
		{
			std::lock_guard<std::mutex> lock(mRunnablesMutex);
			mRunState.mDestructing = true;
			if (!mRunState.mRunning) {
				// If jobs are paused and not running, shutting down now will never complete, so clear runnables
				mRunnables.clear();
			}
		}

		do {
			if (gDebugLogging) LOG(LogDebug) << "Executor service notifying threads of imminent shutdown";
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			mCondition.notify_all();
		} while (!mRunState.IsShutdown());

		if (gDebugLogging) LOG(LogDebug) << "Executor service notified all waiting schedulers to exit immediately";

		for (auto& t : mPoolThreads) {
			if (t.joinable()) {
				t.join();
			}
		}

		mPoolThreads.clear();
	}

	bool ExecutorService::isShutdown() {
		return mRunState.IsShutdown();
	}

	void ExecutorService::startJobs() {
		LOG(LogDebug) << "Start jobs " << mName;
		mRunState.mRunning = true;
		mCondition.notify_all();
	}

	void ExecutorService::pauseJobs() {
		LOG(LogDebug) << "Pause jobs " << mName;
		mRunState.mRunning = false;
	}

	void ExecutorService::clearJobs() {
		LOG(LogDebug) << "Clear jobs " << mName;

		std::lock_guard<std::mutex> lock(mRunnablesMutex);
		mRunState.mRunning = false;
		mRunnables.clear();
	}

	bool ExecutorService::queueJob(std::unique_ptr<Runnable> runnable) {
		{
			if (mRunState.mDestructing) {
				LOG(LogWarning) << "Service is shutdown and waiting for destruction";
				return false;
			}

			std::lock_guard<std::mutex> lock(mRunnablesMutex);
			if (mMaxQueuedJobs > 0 && mRunnables.size() > mMaxQueuedJobs) {
				LOG(LogWarning) << "Too many jobs!";
				return false;
			}
			mRunnables.push_back(std::move(runnable));
			mNumTotalRequests++;
		}

		if (mRunState.mRunning) {
			mCondition.notify_one();
		}

		return true;
	}

	bool ExecutorService::queueJob(std::string_view name, RUN_FUNC work) {
		return queueJob(std::make_unique<Worker>(name, std::move(work)));
	}

	void ExecutorService::workScheduler(int32_t id) {
		mRunState.mNumRunningJobThreads++;
		while (true) {
			if (mRunState.mDestructing) {
				std::unique_lock<std::mutex> lock(mRunnablesMutex);
				if (mRunnables.empty()) {
					break;
				}
			}

			std::unique_ptr<Runnable> runnable = nullptr;
			if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " started";
			if (!mRunState.mRunning) {
				std::unique_lock<std::mutex> lock(mRunnablesMutex);
				if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " is paused";
				mCondition.wait(lock);
			}
			else {
				std::unique_lock<std::mutex> lock(mRunnablesMutex);
				if (mRunnables.empty()) {
					if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " is waiting for work";
					mCondition.wait(lock);
				}
				else {
					auto time = l::string::get_unix_epoch_ms();
					for (uint32_t i = 0; i < mRunnables.size(); i++) {
						if (mRunnables.at(i)->CanRun(time)) {
							runnable = std::move(mRunnables.at(i));
							mRunnables.erase(mRunnables.begin() + i);

							if (gDebugLogging) {
								if (runnable->NumTries() > 0) {
									LOG(LogDebug) << "Scheduler " << id << " picked up requeued(" << runnable->NumTries() << ") job";
								}
								else {
									LOG(LogDebug) << "Scheduler " << id << " picked up new job";
								}
							}
							break;
						}
					}
					lock.unlock();

					if (!runnable) {
						if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " sleeping";
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}
				}
			}

			if (runnable) {
				if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " executes task";
				mRunState.mNumRunningJobs++;
				RunnableResult result = runnable->run(mRunState);
				mRunState.mNumRunningJobs--;
				switch (result) {
				case l::concurrency::RunnableResult::FAILURE:
					if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " task failed";
					runnable.reset();
					break;
				case l::concurrency::RunnableResult::CANCELLED:
					if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " task was cancelled";
					runnable.reset();
					break;
				case l::concurrency::RunnableResult::SUCCESS:
					if (gDebugLogging) LOG(LogDebug) << "Scheduler " << id << " task succeeded";
					mNumCompletedJobs++;
					runnable.reset();
					break;
				case l::concurrency::RunnableResult::REQUEUE_DELAYED:
					runnable->Reschedule();
					if (gDebugLogging) LOG(LogDebug) << "Job '" + runnable->Name() + "' could not run yet and was requeued ";
					queueJob(std::move(runnable));
					break;
				case l::concurrency::RunnableResult::REQUEUE_BACKOFF:
					runnable->Backoff();
					if (!runnable->Failed()) {

						if (gDebugLogging) LOG(LogDebug) << "Job '" + runnable->Name() + "' was delayed and then requeued with backoff";
						queueJob(std::move(runnable));
					}
					else {
						if (gDebugLogging) LOG(LogDebug) << "Job '" + runnable->Name() + "' failed and was cancelled";
						runnable.reset();
					}
					break;
				case l::concurrency::RunnableResult::REQUEUE_IMMEDIATE:
					if (gDebugLogging) LOG(LogInfo) << "Scheduler " << mName << " task was requeued";
					queueJob(std::move(runnable));
					break;
				}
			}
		}
		mRunState.mNumRunningJobThreads--;

		if (gDebugLogging) LOG(LogInfo) << "Scheduler " << mName << " exited";
	}

}
