#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include <optional>
#include <string>

namespace l::math::smooth {
	// source https://iquilezles.org/articles/smoothsteps/

	// Quintic Polynomial - C2 continuity
	template <class V>
	V smoothPolyC2(V x) {
		return x * x * x * (x * (x * static_cast<V>(6.0) - static_cast<V>(15.0)) + static_cast<V>(10.0));
	}

	template <class V>
	V smoothPolyC2Grad(V x) {
		return static_cast<V>(30.0) * x * x * (x * (x - static_cast<V>(2.0)) + static_cast<V>(1.0));
	}

	template <class V>
	V smoothPoly4(V x) {
		V x2 = x * x;
		return static_cast<V>(4.0) * x2 * x - static_cast<V>(3.0) * x2 * x2;
	}

	template <class V>
	V smoothPoly4grad(V x) {
		V x2 = x * x;
		return static_cast<V>(12.0) * x2 - static_cast<V>(12.0) * x2 * x;
	}

	template <class V>
	V smoothPolyh3(V x) {
		V x2 = x * x;
		return static_cast<V>(3.0) * x2 - static_cast<V>(2.0) * x2 * x;
	}

	template <class V>
	V smoothPoly3grad(V x) {
		return static_cast<V>(6.0) * x - static_cast<V>(6.0) * x * x;
	}

	template <class V>
	V smoothRationalC2(V x)
	{
		return x * x * x / (static_cast<V>(3.0) * x * x - static_cast<V>(3.0) * x + static_cast<V>(1.0));
	}

}
