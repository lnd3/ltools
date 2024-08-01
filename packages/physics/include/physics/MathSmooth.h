#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include <optional>
#include <string>

namespace l {
namespace algorithm {

	// Quintic Polynomial - C2 continuity
	template <class V>
	V smoothPolyC2(V x) {
		return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
	}

	template <class V>
	V smoothPolyC2Grad(V x) {
		return 30.0 * x * x * (x * (x - 2.0) + 1.0);
	}

	template <class V>
	V smoothPoly4(V x) {
		V x2 = x * x;
		return 4.0 * x2 * x - 3.0 * x2 * x2;
	}

	template <class V>
	V smoothPoly4grad(V x) {
		V x2 = x * x;
		return 12.0 * x2 - 12.0 * x2 * x;
	}

	template <class V>
	V smootPolyh3(V x) {
		V x2 = x * x;
		return 3.0 * x2 - 2.0 * x2 * x;
	}

	template <class V>
	V smoothPoly3grad(V x) {
		return 6.0 * x - 6.0 * x * x;
	}

	template <class V>
	V smoothRationalC2(V x)
	{
		return x * x * x / (3.0 * x * x - 3.0 * x + 1.0);
	}

}
}
