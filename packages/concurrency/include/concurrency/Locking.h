#pragma once

#include <atomic>
#include <vector>
#include <map>
#include <functional>
#include <mutex>

namespace l::concurrency {
	class SpinLock {
		std::atomic_flag locked = ATOMIC_FLAG_INIT;
	public:
		void lock();
		void unlock();
	};

	template <class T>
	using ActionCallType = std::function<void*(T&)>;

	template <class T>
	class SpinLockedData {
	public:

		SpinLockedData() = default;
		SpinLockedData(SpinLockedData<T>&&) = default;
		SpinLockedData(const SpinLockedData<T>&) = default;
		virtual ~SpinLockedData() = default;

		void action(ActionCallType<T> actionCall) {
			lock.lock();
			actionCall(data);
			lock.unlock();
		}

		SpinLock lock;
		T data;
	};

	template <class T>
	T get(SpinLockedData<std::vector<T>>& l, std::function<T && (std::vector<T>&)> actionCall) {
		l.lock.lock();
		T a = actionCall(l.data);
		l.lock.unlock();
		return a;
	}

	template <class K, class T>
	T get(SpinLockedData<std::map<K, T>>& l, std::function<T && (std::map<K,T>&)> actionCall) {
		l.lock.lock();
		T a = actionCall(l.data);
		l.lock.unlock();
		return a;
	}
}
