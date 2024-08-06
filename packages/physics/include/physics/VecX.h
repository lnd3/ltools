#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <sstream>
#include <cmath>

namespace l {
namespace vec {
#ifndef DEFAULT_SIMD_ALIGNMENT 
#define DEFAULT_SIMD_ALIGNMENT alignas(128)
#endif

	template <class T, class V>
	struct VecApi {
		using type = V;

		VecApi() = default;
		VecApi(VecApi&&) = default;
		VecApi(const VecApi&) = default;
		~VecApi() = default;

		virtual bool operator==(const T&) = 0;

		virtual T& operator=(const T&) = 0;
		virtual T& add(const T&) = 0;
		virtual T& sub(const T&) = 0;
		virtual T& mul(const T&) = 0;
		virtual T& mul(V) = 0;
		virtual T& operator+=(const T&) = 0;
		virtual T& operator-=(const T&) = 0;
		virtual T& operator*=(const T&) = 0;
		//virtual T& operator/=(const T&) = 0; // not very common
		virtual T& operator*=(V) = 0;
		virtual T operator+(const T&) const = 0;
		virtual T operator-(const T&) const = 0;
		virtual T operator*(const T&) const = 0;
		//virtual T operator/(const T&) const = 0; // not very common
		virtual T operator*(V) const = 0;
		virtual T operator/(V) const = 0;
		virtual T operator-() const = 0;

		virtual T cross(const T& v) const = 0;
		virtual V dot(const T& v) const = 0;
		virtual V sqr() const = 0;
		virtual V length() const = 0;

		virtual T fract(V gridSize) const = 0;
		virtual T floor(V gridSize) const = 0;

		virtual std::string to_string() const = 0;
	};

	template <class T>
	requires std::floating_point<T>
	struct Data2 : public VecApi<Data2<T>, T> {
		using type = T;

		Data2() : x1(0.0), x2(0.0) {}
		Data2(T _x1, T _x2) : x1(_x1), x2(_x2) {}

		T x1, x2;

		bool operator==(const Data2& v) override {
			return x1 == v.x1 && x2 == v.x2;
		}

		Data2& operator=(const Data2& v) override {
			this->x1 = v.x1;
			this->x2 = v.x2;
			return *this;
		}
			
		Data2& add(const Data2& v) override {
			x1 += v.x1;
			x2 += v.x2;
			return *this;
		}

		Data2& sub(const Data2& v) override {
			x1 -= v.x1;
			x2 -= v.x2;
			return *this;
		}

		Data2& mul(const Data2& v) override {
			x1 *= v.x1;
			x2 *= v.x2;
			return *this;
		}

		Data2& mul(T f) override {
			x1 *= f;
			x2 *= f;
			return *this;
		}

		Data2& operator+=(const Data2& v) override {
			return add(v);
		}

		Data2& operator-=(const Data2& v) override {
			return sub(v);
		}

		Data2& operator*=(const Data2& v) override {
			return mul(v);
		}

		Data2& operator*=(T f) override {
			return mul(f);
		}

		Data2 operator+(const Data2& v) const override {
			return Data2(x1 + v.x1, x2 + v.x2);
		}

		Data2 operator-(const Data2& v) const override {
			return Data2(x1 - v.x1, x2 - v.x2);
		}

		Data2 operator*(const Data2& v) const override {
			return Data2(x1 * v.x1, x2 * v.x2);
		}

		Data2 operator*(T f) const override {
			return Data2(x1 * f, x2 * f);
		}

		Data2 operator/(T f) const override {
			return Data2(x1 / f, x2 / f);
		}

		Data2 operator-() const override {
			return Data2(- x1, - x2);
		}

		Data2 cross(const Data2& v) const override {
			return Data2(0, 0);
		}

		T dot(const Data2& v) const override {
			return x1 * v.x1 + x2 * v.x2;
		}

		T sqr() const override {
			return x1 * x1 + x2 * x2;
		}

		T length() const override {
			return sqrt(x1 * x1 + x2 * x2);
		}

		Data2 fract(T gridSize = 1) const override {
			auto x1scaled = x1 / gridSize;
			auto x2scaled = x2 / gridSize;
			return Data2(x1scaled - ::floor(x1scaled), x2scaled - ::floor(x2scaled));
		}

		Data2 floor(T gridSize = 1) const override {
			return Data2(::floor(x1 / gridSize), ::floor(x2 / gridSize));
		}

		std::string to_string() const override {
			std::stringstream out{};
			out << "[";
			out << std::to_string(x1) << ",";
			out << std::to_string(x2) << "]";
			return out.str();
		}
	};

	template <class T>
	requires std::floating_point<T>
	struct Data2x2 {
		T x11, x12, x21, x22; // row 1 then 2
	};

	template <class T> using LinearPosition2 = Data2<T>;
	template <class T> using AngularPosition2 = Data2<T>;

	template <class T> using LinearVelocity2 = Data2<T>;
	template <class T> using AngularVelocity2 = Data2<T>;

	template <class T> using LinearForce2 = Data2<T>;
	template <class T> using AngularForce2 = Data2<T>;

	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Position2 {
		LinearPosition2<T> x; // Linear position
		AngularPosition2<T> q; // Angular position (orientation)
	};
	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Velocity2 {
		LinearVelocity2<T> v; // Linear velocity
		AngularVelocity2<T> w; // Angular velocity
	};
	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Force2 {
		LinearForce2<T> f; // Linear force
		AngularForce2<T> t; // Angular force (torque)
	};

/***********************************************************************/
	template <class T>
	requires std::floating_point<T>
	struct Data4 : public VecApi<Data4<T>, T> {
		using type = T;

		Data4() : x1(0.0), x2(0.0), x3(0.0), x4(0.0) {}
		Data4(T _x1, T _x2, T _x3, T _x4) : x1(_x1), x2(_x2), x3(_x3), x4(_x4) {}

		T x1, x2, x3, x4;

		std::array<T, 4> data() {
			return *reinterpret_cast<T[4]>(&x1);
		}

		bool operator==(const Data4& v) override {
			return x1 == v.x1 && x2 == v.x2 && x3 == v.x3 && x4 == v.x4;
		}

		bool operator!=(const Data4& v) {
			return x1 != v.x1 || x2 != v.x2 || x3 != v.x3 || x4 != v.x4;
		}

		void zero() {
			x1 = x2 = x3 = x4 = 0.0;
		}

		Data4& operator=(const Data4& v) override {
			this->x1 = v.x1;
			this->x2 = v.x2;
			this->x3 = v.x3;
			this->x4 = v.x4;
			return *this;
		}

		Data4& add(const Data4& v) override {
			x1 += v.x1;
			x2 += v.x2;
			x3 += v.x3;
			x4 += v.x4;
			return *this;
		}

		Data4& sub(const Data4& v) override {
			x1 -= v.x1;
			x2 -= v.x2;
			x3 -= v.x3;
			x4 -= v.x4;
			return *this;
		}

		Data4& mul(const Data4& v) override {
			x1 *= v.x1;
			x2 *= v.x2;
			x3 *= v.x3;
			x4 *= v.x4;
			return *this;
		}

		Data4& mul(T f) override {
			x1 *= f;
			x2 *= f;
			x3 *= f;
			x4 *= f;
			return *this;
		}

		Data4& operator+=(const Data4& v) override {
			return add(v);
		}

		Data4& operator-=(const Data4& v) override {
			return sub(v);
		}

		Data4& operator*=(const Data4& v) override {
			return mul(v);
		}

		Data4& operator*=(T f) {
			return mul(f);
		}

		Data4 operator+(const Data4& v) const override {
			return Data4(x1 + v.x1, x2 + v.x2, x3 + v.x3, x4 + v.x4);
		}

		Data4 operator-(const Data4& v) const override {
			return Data4(x1 - v.x1, x2 - v.x2, x3 - v.x3, x4 - v.x4);
		}

		Data4 operator*(const Data4& v) const override {
			return Data4(x1 * v.x1, x2 * v.x2, x3 * v.x3, x4 * v.x4);
		}

		Data4 operator*(T f) const override {
			return Data4(x1 * f, x2 * f, x3 * f, x4 * f);
		}

		Data4 operator/(T f) const override {
			return Data4(x1 / f, x2 / f, x3 / f, x4 / f);
		}

		Data4 operator-() const override {
			return Data4(-x1, -x2, -x3, -x4);
		}

		Data4 cross(const Data4& v) const override {
			return Data4(
				x2 * v.x3 - x3 * v.x2, 
				x3 * v.x1 - x1 * v.x3, 
				x1 * v.x2 - x2 * v.x1, 
				0.0);
		}

		T dot(const Data4& v) const override {
			return x1 * v.x1 + x2 * v.x2 + x3 * v.x3 + x4 * v.x4;
		}

		T sqr() const override {
			return x1 * x1 + x2 * x2 + x3 * x3 + x4 * x4;
		}

		T length() const override {
			return sqrt(x1 * x1 + x2 * x2 + x3 * x3 + x4 * x4);
		}

		Data4 fract(T gridSize) const override {
			T x1scaled = x1 / gridSize;
			T x2scaled = x2 / gridSize;
			T x3scaled = x3 / gridSize;
			T x4scaled = x4 / gridSize;
			return Data4(x1scaled - ::floor(x1scaled), x2scaled - ::floor(x2scaled), x3scaled - ::floor(x3scaled), x4scaled - ::floor(x4scaled));
		}

		Data4 floor(T gridSize) const override {
			return Data4(::floor(x1 / gridSize), ::floor(x2 / gridSize), ::floor(x3 / gridSize), ::floor(x4 / gridSize));
		}

		std::string to_string() const override {
			std::stringstream out{};
			out << "[";
			out << std::to_string(x1) << ",";
			out << std::to_string(x2) << ",";
			out << std::to_string(x3) << ",";
			out << std::to_string(x4) << "]";
			return out.str();
		}
	};

	template <class T>
	struct Data4x4 {
		T x11, x12, x13, x14, x21, x22, x23, x24, x31, x32, x33, x34, x41, x42, x43, x44;
	};

	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Position4 {
		Data4<T> x; // Linear position
		Data4<T> q; // Angular position (orientation)
	};
	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Velocity4 {
		Data4<T> v; // Linear velocity
		Data4<T> w; // Angular velocity
	};
	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Force4 {
		Data4<T> f; // Linear force
		Data4<T> t; // Angular force (torque)
	};
	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Jerk4 {
		Data4<T> linearJerk; // Linear jerk
		Data4<T> angularJerk; // Angular jerk
	};
	template <class T>
	struct DEFAULT_SIMD_ALIGNMENT Snap4 {
		Data4<T> linearSnap; // Linear snap
		Data4<T> angularSnap; // Angular snap
	};


};

}

template<class V>
requires std::floating_point<V>
std::string to_string(l::vec::Data2<V> data) {
	std::stringstream out;
	out << "[";
	out << std::to_string(data.x1) << ",";
	out << std::to_string(data.x2) << "]";
	return out.str();
}

template<class V>
requires std::floating_point<V>
std::string to_string(l::vec::Data4<V> data) {
	std::stringstream out;
	out << "[";
	out << std::to_string(data.x1) << ",";
	out << std::to_string(data.x2) << ",";
	out << std::to_string(data.x3) << ",";
	out << std::to_string(data.x4) << "]";
	return out.str();
}


