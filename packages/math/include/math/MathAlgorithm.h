#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include <optional>
#include <string>
#include <algorithm>
#include <set>
#include <cmath>
#include <cstring>
#include "logging/LoggingAll.h"
#include <math.h>

#include "math/MathFunc.h"

namespace l::math::algorithm {

	uint64_t pairIndex32(uint32_t i, uint32_t j);
	uint32_t pairIndex16(uint16_t i, uint16_t j);
	uint32_t encodeFloat(const float newPos);
	float decodeFloat(uint32_t ir);

	template<class T, class U>
	T convert(U data) {
		const char* srcPtr = reinterpret_cast<const char*>(&data);
		T dst;
		memcpy(&dst, srcPtr, sizeof(T));
		return dst;
	}

	template <class T>
	void reflect(T& d, const T& n) {
		auto t = d.dot(n) * 2.0;
		auto v = n * t / n.sqr();
		d.sub(v);
	};

	template<class T>
	uint32_t binary_search(const std::vector<T>& elements, const T& data, int32_t minIndex = 1, int32_t maxIndex = INT32_MAX) {
		uint32_t L = static_cast<uint32_t>(minIndex < 0 ? 0 : minIndex);
		uint32_t R = static_cast<uint32_t>((maxIndex < elements.size() ? maxIndex : elements.size()) - 1);

		while (L <= R) {
			uint32_t m = static_cast<uint32_t>(floor((L + R) / 2.0));
			auto& e = elements.at(static_cast<size_t>(m));
			if (e < data) {
				L = m + 1;
			}
			else if (data < e) {
				R = m - 1;
			}
			else {
				return m;
			}
		}
		return 0;
	}

	template <class T>
	T bisect(T a, T b, T tolerance, int iterations, std::function<T(T)> eval) {
		int n = 0;
		T c = 0.0;
		while (n < iterations) {
			c = (a + b) / 2.0;

			T cEval = eval(c);
			LOG(LogDebug) << "bisect iteration " << n << "(" << iterations << ") val: " << c << "(convergence: " << cEval << ")";
			if (cEval == 0.0 || abs(cEval) < tolerance) {
				return c;
			}
			n++;
			T aEval = eval(a);
			if (l::math::functions::samesign(cEval, aEval) > 0.0f) {
				a = c;
			}
			else {
				b = c;
			}
		}
		return c;
	}

	template <class T>
	std::optional<std::pair<T, T>> solve_quad_eq(T p, T q) {
		T x1 = -p / 2.0;
		T t = x1 * x1 - q;
		if (t < 0.0) {
			return std::nullopt;
		}
		T x2 = sqrt(t);
		return std::make_pair(x1 + x2, x1 - x2);
	}

	template <class T>
	std::optional<std::pair<T, T>> solve_quad_eq(T a, T b, T c) {
		T p = b / a;
		T q = c / a;
		T x1 = -p / 2.0;
		T t = x1 * x1 - q;
		if (t < 0.0) {
			return std::nullopt;
		}
		T x2 = sqrt(t);
		return std::make_pair(x1 + x2, x1 - x2);
	}

	template <class T>
	void gauss_seidel(std::vector<std::vector<T>> A, std::vector<T> b, std::vector<T>& out, int iterations) {

		do {
			int n = A.size();
			for (int i = 0; i < n; i++) {
				T lambda = 0;
				for (int j = 0; j < n; j++) {
					if (j != i) {
						lambda = lambda + A[i][j] * out[j];
					}
				}
				out[i] = (b[i] - lambda) / A[i][i];
			}

			// Check if convergence is reached
			if (iterations-- <= 0) {
				break;
			}
		} while(true);
	}

	template<class T>
	void set_intersection(std::set<T>& dst, const std::set<T>& set1, const std::set<T>& set2, const std::set<T>& set3) {
		auto it1 = set1.begin();
		auto it2 = set2.begin();
		auto it3 = set3.begin();
		while (it1 != set1.end() && it2 != set2.end() && it3 != set3.end())
		{
			if (*it1 < *it2) {
				++it1;
			}
			else if (*it2 < *it3) {
				++it2;
			}
			else if (*it3 < *it1) {
				++it3;
			}
			else {
				dst.emplace(*it1);
				++it1;
				++it2;
				++it3;
			}
		}

	}
}
