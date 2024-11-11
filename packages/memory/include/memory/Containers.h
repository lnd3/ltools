#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include "logging/Log.h"
#include "meta/Reflection.h"

namespace l::container {

	template<typename T>
	std::vector<T> vector_extract(std::vector<T>& v, size_t i, size_t count) {
		ASSERT(i < v.size() && count <= v.size());
		std::vector<T> dst;
		dst.insert(dst.end(), std::make_move_iterator(v.begin() + i), std::make_move_iterator(v.begin() + i + count));
		v.erase(v.begin() + i, v.begin() + i + count);
		return dst;
	};

}
