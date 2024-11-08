#pragma once

#include <mutex>

namespace l::concurrency {

	template<class T>
	class ObjectLock {
	public:
		ObjectLock() = default;
		ObjectLock(std::mutex& mutex, T* object) :
			mLock(mutex, std::adopt_lock),
			mObject(object)
		{}

		~ObjectLock() = default;

		ObjectLock& operator=(ObjectLock&& other) {
			if (this != &other) {
				mLock = std::move(other.mLock);
				mObject = other.mObject;
				other.mObject = nullptr;
			}
			return *this;
		}

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
		T* mObject = nullptr;
	};

}
