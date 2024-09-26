#pragma once

#include <vector>
#include <functional>
#include <optional>
#include <string>
#include <algorithm>
#include <utility>
#include <math.h>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

namespace l::math::functions {

	template<class T>
	auto abs(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return fabsf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return fabs(val);
			}
		}
		else if constexpr (std::is_integral_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return ::abs(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return llabs(val);
			}
		}
	}

	template<class T>
	auto equal(T a, T b, T accuracy = static_cast<T>(0.0001)) {
		return functions::abs(a - b) < accuracy;
	}

	template <class T>
	auto samesign(T a, T b) {
		return a * b >= static_cast<T>(0.0);
	}

	template<class T>
	void swap(T& val1, T& val2) {
		T tmp = std::move(val1);
		val1 = std::move(val2);
		val2 = std::move(tmp);
	}

	template<class T>
	auto min2(T val1, T val2) {
		return val1 < val2 ? val1 : val2;
	}

	template<class T>
	auto max2(T val1, T val2) {
		return val1 > val2 ? val1 : val2;
	}

	template<class T>
	auto clamp(T val, T min, T max) {
		return val < min ? min : val > max ? max : val;
	}

	template<class T>
	requires std::is_floating_point_v<T>
	auto floor(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return floorf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::floor(val);
			}
		}
	}

	template<class T>
	auto sqrt(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return sqrtf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::sqrt(val);
			}
		}
	}

	template<class T>
	auto pow(T val, T exp) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return powf(val, exp);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::pow(val, exp);
			}
		}
	}

	template<class T>
	auto exp(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return expf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::exp(val);
			}
		}
	}

	template<class T>
	auto sin(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return sinf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::sin(val);
			}
		}
	}

	template<class T>
	auto cos(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return cosf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::cos(val);
			}
		}
	}

	template<class T>
	auto tan(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return tanf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::tan(val);
			}
		}
	}

	template<class T>
	auto asin(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return asinf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::asin(val);
			}
		}
	}

	template<class T>
	auto acos(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return acosf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::acos(val);
			}
		}
	}

	template<class T>
	auto atan(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return atanf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return ::atan(val);
			}
		}
	}

	template<class T>
	auto mod(T val, T mod) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return fmodf(val, mod);
			}
			else if constexpr (sizeof(T) == 8) {
				return fmod(val, mod);
			}
		}
	}

	template<class T>
	auto round(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return roundf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return roundl(val);
			}
		}
	}

	template<class T>
	auto log(T val) {
		if constexpr (std::is_floating_point_v<T>) {
			if constexpr (sizeof(T) == 4) {
				return logf(val);
			}
			else if constexpr (sizeof(T) == 8) {
				return logl(val);
			}
		}
	}

	// source https://iquilezles.org/articles/functions/

	// Almost Identity(I)
	// Imagine you don't want to modify a signal unless it's drops to zero or close to it, in which case you want to replace the value with a small possitive constant.Then, rather than clamping the valueand introduce a discontinuity, you can smoothly blend the signal into the desired clipped value.So, let m be the threshold(anything above m stays unchanged), and n the value things will take when the signal is zero.Then, the following function does the soft clipping(in a cubic fashion):
	template<class T>
	auto almostIdentity(T x, T m, T n)
	{
		if (x > m) return x;
		const T a = static_cast<T>(2.0) * n - m;
		const T b = static_cast<T>(2.0) * m - static_cast<T>(3.0) * n;
		const T t = x / m;
		return (a * t + b) * t * t + n;
	}

	// Almost Identity(II)
	// A different way to achieve a near identity is through the square root of a biased square.I saw this technique first in a shader by user "omeometo" in Shadertoy.This approach can be a bit slower than the cubic above, depending on the hardware, but I find myself using it a lot these days.While it has zero derivative, it has a non - zero second derivative, so keep an eye in case it causes problems in your application.
	// An extra nice thing is that this function can be used, unaltered, as an smooth - abs() function, which comes handy for symmetric funtions such as mirrored SDFs.
	template<class T>
	auto almostIdentity2(T x, T n)
	{
		return functions::sqrt(x * x + n);
	}

	// Almost Unit Identity
	// This is a near - identiy function that maps the unit interval into itself.It is the cousin of smoothstep(), in that it maps 0 to 0, 1 to 1, and has a 0 derivative at the origin, just like smoothstep.However, instead of having a 0 derivative at 1, it has a derivative of 1 at that point.It's equivalent to the Almost Identiy above with n=0 and m=1. Since it's a cubic just like smoothstep() it is very fast to evaluate :
	template<class T>
	auto almostUnitIdentity(T x)
	{
		return x * x * (static_cast<T>(2.0) - x);
	}

	// Smoothstep Integral
	// If you use smoothstep for a velocity signal(say, you want to smoothly accelerate a stationary object into constant velocity motion), you need to integrate smoothstep() over time in order to get the actual position of value of the animation.The function below is exactly that, the position of an object that accelerates with smoothstep.Note it's derivative is never larger than 1, so no decelerations happen.
	template<class T>
	auto integralSmoothstep(T x, T t)
	{
		if (x >= t) return x - t * static_cast<T>(0.5);
		T f = x / t;
		return f * f * f * (t - x * static_cast<T>(0.5));
	}

	// Exponential Impulse
	// Impulses are great for triggering behaviours or making envelopes for music or animation.Baiscally, for anything that grows fastand then decays slowly.The following is an exponential impulse function.Use k to control the stretching of the function.Its maximum, which is 1, happens at exactly x = 1 / k.
	template<class T>
	auto expImpulse(T x, T k) 
	{
		const T h = k * x;
		return h * functions::exp(static_cast<T>(1.0) - h);
	}

	// Polynomial Impulse
	// Another impulse function that doesn't use exponentials can be designed by using polynomials. Use k to control falloff of the function. For example, a quadratic can be used, which peaks at x = sqrt(1/k).
	template<class T>
	auto quaImpulse(T k, T x)
	{
		return static_cast<T>(1.0) * functions::sqrt(k) * x / (static_cast<T>(1.0) + k * x * x);
	}

	// You can easily generalize it to other powers to get different falloff shapes, where n is the degree of the polynomial :
	template<class T>
	auto polyImpulse(T k, T n, T x)
	{
		return (n / (n - static_cast<T>(1.0))) * functions::pow((n - static_cast<T>(1.0)) * k, static_cast<T>(1.0) / n) * x / (static_cast<T>(1.0) + k * functions::pow(x, n));
	}

	// Sustained Impulse
	// Similar to the previous, but it allows for control on the width of attack(through the parameter "k") and the release(parameter "f") independently.Also, the impulse releases at a value of 1 instead of 0.
	template<class T>
	auto expSustainedImpulse(T x, T f, T k)
	{
		auto s = functions::max2(x - f, static_cast<T>(0.0));
		auto a = x * x / (f * f);
		auto b = static_cast<T>(1.0) + (static_cast<T>(2.0) / f) * s * functions::exp(-k * s);
		return functions::min2(a, b);
	}

	// Cubic Pulse
	// Chances are you found yourself doing smoothstep(c - w, c, x) - smoothstep(c, c + w, x) very often.I do, for example when I need to isolate some features in a signal.For those cases, this cubicPulse() below is my new friendand will be yours too soon.Bonus - you can also use it as a performant replacement for a gaussian.
	template<class T>
	auto cubicPulse(T c, T w, T x)
	{
		x = functions::abs(x - c);
		if (x > w) return static_cast<T>(0.0);
		x /= w;
		return static_cast<T>(1.0) - x * x * (static_cast<T>(3.0) - static_cast<T>(2.0) * x);
	}

	// Exponential Step
	// A natural attenuation is an exponential of a linearly decaying quantity : yellow curve, exp(-x).A gaussian, is an exponential of a quadratically decaying quantity : light green curve, exp(-x2).You can generalizeand keep increasing powers, and get a sharperand sharper s - shaped curves.For really high values of n you can approximate a perfect step().If you want such step to transition at x = a, like in the graphs to the right, you can set k = a - nln 2.
	template<class T>
	auto expStep(T x, T k, T n)
	{
		return functions::exp(-k * functions::pow(x, n));
	}

	// Gain
	// Remapping the unit interval into the unit interval by expanding the sides and compressing the center, and keeping 1 / 2 mapped to 1 / 2, that can be done with the gain() function.This was a common function in RSL tutorials(the Renderman Shading Language).k = 1 is the identity curve, k < 1 produces the classic gain() shape, and k>1 produces "s" shaped curces.The curves are symmetric(and inverse) for k = a and k = 1 / a.
	template<class T>
	auto gain(T x, T k)
	{
		const T a = static_cast<T>(0.5) * functions::pow(static_cast<T>(2.0) * ((x < static_cast<T>(0.5)) ? x : static_cast<T>(1.0) - x), k);
		return (x < static_cast<T>(0.5)) ? a : static_cast<T>(1.0) - a;
	}

	// Parabola
	// A nice choice to remap the 0..1 interval into 0..1, such that the corners are mapped to 0 and the center to 1. You can then rise the parabolar to a power k to control its shape.
	template<class T>
	auto parabola(T x, T k)
	{
		return functions::pow(4.0 * x * (static_cast<T>(1.0) - x), k);
	}

	// Power curve
	// This is a generalziation of the Parabola() above.It also maps the 0..1 interval into 0..1 by keeping the corners mapped to 0. But in this generalziation you can control the shape one either side of the curve, which comes handy when creating leaves, eyes, and many other interesting shapes.
	template<class T>
	auto pcurve(T x, T a, T b)
	{
		const T k = functions::pow(a + b, a + b) / (functions::pow(a, a) * functions::pow(b, b));
		return k * functions::pow(x, a) * functions::pow(1.0 - x, b);
	}

	// Sinc curve
	// A phase shifted sinc curve can be useful if it starts at zeroand ends at zero, for some bouncing behaviors(suggested by Hubert - Jan).Give k different integer values to tweak the amount of bounces.It peaks at 1.0, but that take negative values, which can make it unusable in some applications.
	template<class T>
	auto sinc(T x, T k)
	{
		const T a = math::constants::PI * (k * x - static_cast<T>(1.0));
		return functions::sin(a) / a;
	}

	template<class T>
	auto sigmoid(T x, T k) {
		return static_cast<T>(static_cast<T>(1.0) / (static_cast<T>(1.0) + functions::exp(-x * k)));
	}

	template<class T>
	auto sigmoidFast(T x) {
		if (x < static_cast<T>(1.0) && x > static_cast<T>(-1.0)) {
			return static_cast<T>(x * (static_cast<T>(1.5) - static_cast<T>(0.5) * x * x));
		}
		else {
			return static_cast<T>(x > static_cast<T>(0.0) ? static_cast<T>(1.0) : static_cast<T>(-1.0));
		}
	}
}
