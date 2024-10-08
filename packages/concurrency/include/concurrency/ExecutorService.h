#pragma once

#include "logging/Log.h"

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <functional>

namespace l::concurrency {

	class RunState {
	public:
		std::atomic_bool mRunning = false;
		std::atomic_bool mDestructing = false;
		std::atomic_int32_t mNumRunningJobs = 0;
		std::atomic_int32_t mNumRunningJobThreads = 0;

		bool IsShuttingDown() const;
		bool IsPaused() const;
		bool HasRunningJobs() const;
		bool HasRunningJobThreads() const;
		bool IsShutdown() const;
	};



	enum class RunnableResult {
		SUCCESS,
		FAILURE,
		REQUEUE_IMMEDIATE,
		REQUEUE_DELAYED,
		REQUEUE_BACKOFF,
		CANCELLED
	};

	using RUN_FUNC = std::function<RunnableResult(const RunState&)>;



	class Runnable {
	public:
		Runnable() : mName("Undefined"), mTries(0), mNextTry(0), mMaxTries(10) {}
		Runnable(const char* name, int32_t maxTries = 10) :
			mName(name), 
			mTries(0),
			mNextTry(0),
			mMaxTries(maxTries < 1 ? 1 : maxTries)
		{}
		Runnable(std::string_view name, int32_t maxTries = 10) :
			mName(name), 
			mTries(0),
			mNextTry(0),
			mMaxTries(maxTries < 1 ? 1 : maxTries)
		{}
		Runnable(Runnable&& other) noexcept {
			mName = other.mName;
			mTries = other.mTries;
			mNextTry = other.mNextTry;
			mMaxTries = other.mMaxTries;
		}
		Runnable& operator=(const Runnable& other) noexcept {
			mName = other.mName;
			mTries = other.mTries;
			mNextTry = other.mNextTry;
			mMaxTries = other.mMaxTries;
			return *this;
		}
		virtual ~Runnable() {}

		std::string Name() const;
		bool CanRun(int64_t time);
		void Reschedule();
		void Backoff();
		bool Failed();
		int32_t NumTries();
		virtual RunnableResult run(const RunState&);
	protected:
		std::string mName;
		int32_t mTries;
		int64_t mNextTry;
		int32_t mMaxTries;
	};



	class Worker : public Runnable {
	public:
		Worker(std::string_view name, RUN_FUNC work, int32_t maxTries = 10) : Runnable(name, maxTries), mWork(work) {}
		Worker(Worker&&) = default;
		Worker(const Worker&) = default;
		virtual ~Worker() override {
			//LOG(LogDebug) << "Destroying " << mName;
		}

		RunnableResult run(const RunState& state) override;
	private:
		RUN_FUNC mWork;
	};



	class ExecutorService {
	public:

		std::atomic_bool gDebugLogging = false;
		static const uint32_t gInfinite = 0;

		ExecutorService(std::string name = "", int32_t numThreads = 10, uint32_t maxQueuedJobs = gInfinite) :
			mName(name), 
			mNumTotalRequests(0), 
			mNumCompletedJobs(0),
			mMaxQueuedJobs(maxQueuedJobs),
			mRunState() {
			for (int32_t i = 0; i < numThreads; i++) {
				mPoolThreads.push_back(std::thread(std::bind(&ExecutorService::workScheduler, this, i)));
			}
		}
		~ExecutorService() {
			shutdown();
		}

		int32_t numJobs();
		int32_t numTotalJobs();
		int32_t numCompletedJobs();
		bool isShuttingDown();
		bool isShutdown();

		void shutdown();
		void startJobs();
		void pauseJobs();
		void clearJobs();
		bool queueJob(std::unique_ptr<Runnable> runnable);
		bool queueJob(std::string_view name, RUN_FUNC work);
	private:
		void workScheduler(int32_t id);

		std::string mName{};
		std::atomic_int32_t mNumTotalRequests;
		std::atomic_int32_t mNumCompletedJobs;
		std::atomic_uint32_t mMaxQueuedJobs;

		RunState mRunState;

		std::vector<std::thread> mPoolThreads;

		std::condition_variable mCondition;
		std::mutex mRunnablesMutex;
		std::vector<std::unique_ptr<Runnable>> mRunnables;
	};

}
