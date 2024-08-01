#pragma once

#include <atomic>

#include "concurrency/Locking.h"

namespace l::concurrency {
	void SpinLock::lock() {
		while (locked.test_and_set(std::memory_order_acquire)) { ; }
	}
	void SpinLock::unlock() {
		locked.clear(std::memory_order_release);
	}
}
