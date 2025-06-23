#pragma once

#include <type_traits>

namespace l {
namespace meta{
	template <class V, class T>
	using IsDerived = std::enable_if_t<std::is_base_of_v<V, T>>;

	template<class T>
	using enable_if_trivial_t = std::enable_if_t<std::is_trivially_constructible_v<T>>;

	template<class T>
	using enable_if_floating_point_t = std::enable_if_t<std::is_floating_point_v<T>>;

	template<typename V, typename T>
	concept DerivedFrom = std::derived_from<V, T>;
}
}