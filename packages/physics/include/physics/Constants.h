#pragma once

#include <cmath>

namespace l {
namespace constants {
	// Newtonian constant of gravitation 6.67430(15)x10-11 m3 kg-1 s-2
	const double G = 6.674 * std::pow(10.0, -11.0);

	// speed of light in vacuum 299792458 m s-1
	const double c = 299792458;

	// Planck constant 6.62607015x10-34 J Hz-1
	const double h = 6.62607015 * std::pow(10.0, -34.0);

	const double pi = 3.14159265359;
	const float pi_f = 3.14159265359f;

	// Minkowski space time distance https://www.youtube.com/watch?v=fvqXshyuvOg
	// Basically objects are 5 dimensional and the distance between objects are
	// intrinsically larger then the measured/observed distance
	// dt - observed time difference between events
	// w - experienced relativistic time difference between locations
	template<class T>
	T minkowskiDistanceSquared(T dt, T w, T dx, T dy, T dz) {
		return c * c * dt * dt - w * w - dx * dx - dy * dy - dz * dz;
	}

}
}
