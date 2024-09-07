#pragma once

#include <cmath>

namespace l::math::constants {
	// Newtonian constant of gravitation 6.67430(15)x10-11 m3 kg-1 s-2
	const double G = 6.674 * std::pow(10.0, -11.0);

	// Speed of light in vacuum 299792458 m s-1
	const double c = 299792458;

	// Planck's constant 6.62607015x10-34 J Hz-1
	const double h = 6.62607015 * std::pow(10.0, -34.0);

	// PI
	const double PI = 3.141592653589793;
	const float PI_f = 3.141592653589793f;

	// euler's number e
	const double E = 2.718281828459045;
	const float E_f = 2.718281828459045f;

	// phi
	const double PHI = 1.618033988749895;
	const double PHI_f = 1.618033988749895f;

	// Minkowski space time distance https://www.youtube.com/watch?v=fvqXshyuvOg
	// Basically objects are 5 dimensional and the distance between objects are
	// intrinsically larger then the measured/observed distance
	// dt - observed time difference between events
	// w - experienced relativistic time difference between locations
	template<class T>
	T minkowskiDistanceSquared(T dt, T w, T dx, T dy, T dz) {
		return c * c * dt * dt - w * w - dx * dx - dy * dy - dz * dz;
	}

	const float FLTMAX = 3.402823466e+38F;
	const float FLTMIN = 1.175494351e-38F;
	const int32_t INTMAX = 2147483647;
	const int32_t INTMIN = (-2147483647 - 1);
}
