#pragma once

#include "storage/SequentialCache.h"

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <map>
#include <mutex>
#include <memory>
#include <optional>


namespace l::filecache {

	template<class... T>
	class CacheGroup {
	public:
		const size_t size = sizeof...(T);

		using cbType = std::function<bool(int32_t, int32_t, SequentialCache<T>*...)>;

	};


}