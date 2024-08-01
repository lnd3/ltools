#pragma once

#include <mutex>

namespace l::concurrency {

	template<class T>
	class ObjectLock {
	public:
		ObjectLock(std::mutex& mutex, T* object) :
			mLock(mutex, std::adopt_lock),
			mObject(object)
		{}
		~ObjectLock() = default;

		void reset() {
			mObject = nullptr;
			mLock.unlock();
		}

		bool valid() {
			return mObject != nullptr;
		}

		T* operator->() {
			return mObject;
		}

		T& operator*() {
			return *mObject;
		}

	protected:
		std::unique_lock<std::mutex> mLock;
		T* mObject;
	};

}
