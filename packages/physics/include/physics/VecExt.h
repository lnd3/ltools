#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <sstream>

namespace l {
namespace vec {

	template<class T>
	T InnerProduct(std::span<T> src1, std::span<T> src2) {
		T result = 0.0;
		auto it1 = src1.begin();
		auto it2 = src2.begin();

		for (; it1 != src1.end() && it2 != src2.end(); it1++, it2++) {
			result += (*it1) * (*it2);
		}
		return result;
	}

	template<class T>
	T Magnitude(std::span<T> src) {
		T result = 0.0;
		auto it = src.begin();

		for (; it != src.end(); it++) {
			result += (*it) * (*it);
		}

		return result;
	}

	template<class T>
	void Scale(std::span<T> dst, std::span<T> src, T scale) {
		auto it1 = dst.begin();
		auto it2 = src.begin();
		for (; it1 != dst.end() && it2 != src.end(); it1++, it2++) {
			*it1 = (*it2) * scale;
		}
	}

	template<class T>
	void Subtract(std::span<T> dst, std::span<T> src1, std::span<T> src2) {
		auto it1 = dst.begin();
		auto it2 = src1.begin();
		auto it3 = src2.begin();
		for (; it1 != dst.end() && it2 != src1.end() && it3 != src2.end(); it1++, it2++, it3++) {
			*it1 = *it2 - *it3;
		}
	}

	template<class T>
	void Normalize(std::span<T> dst, std::span<T> src) {
		T l = Magnitude<T>(src);
		if (l > 0) {
			l = 1 / l;
		}
		Scale<T>(dst, src, l);
	}

	template<class T>
	void Cross(std::span<T> dst, std::span<T> src1, std::span<T> src2) {
		auto it1 = dst.begin();
		auto it2 = src1.begin();
		auto it3 = src2.begin();

		T x1 = *it2++;
		T y1 = *it2++;
		T z1 = *it2;
		T x2 = *it3++;
		T y2 = *it3++;
		T z2 = *it3;

		T cx = y1 * z2 - z1 * y2;
		T cy = z1 * x2 - x1 * z2;
		T cz = x1 * y2 - y1 * x2;

		*it1++ = cx;
		*it1++ = cy;
		*it1 = cz;
	}

	template<class T>
	bool IsZeroVector(std::span<T> src) {
		for (auto it : src) {
			if (it != 0.0) {
				return false;
			}
		}
		return true;
	}
}
}


