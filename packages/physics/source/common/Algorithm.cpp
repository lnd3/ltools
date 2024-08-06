
#include "physics/Algorithm.h"

namespace l {
namespace algorithm {

	uint64_t pairIndex32(uint32_t i, uint32_t j) {
		/// Make sure index is order invariant
		if (j < i) {
			return j | (static_cast<uint64_t>(i) << 32);
		}
		return i | (static_cast<uint64_t>(j) << 32);
	}

	uint32_t pairIndex16(uint16_t i, uint16_t j) {
		/// Make sure index is order invariant
		if (j < i) {
			return j | (static_cast<uint32_t>(i) << 16);
		}
		return i | (static_cast<uint32_t>(j) << 16);
	}

	uint32_t encodeFloat(const float newPos) {
		//we may need to check on -0 and 0
		//But it should make no practical difference.
		uint32_t ir = convert<uint32_t>(newPos);

		if (ir & 0x80000000) { //negative?
			ir = ~ir;//reverse sequence of negative numbers
		}
		else {
			ir |= 0x80000000; // flip sign
		}
		return ir;
	}

	float decodeFloat(uint32_t ir) {
		uint32_t rv;
		if (ir & 0x80000000) { //positive?
			rv = ir & ~0x80000000; //flip sign
		}
		else {
			rv = ~ir; //undo reversal
		}

		return convert<float>(rv);
	}

	float q_rsqrt(float number) {
		long i;
		float x2, y;
		const float threehalfs = 1.5F;

		x2 = number * 0.5F;
		y = number;
		i = convert<long>(y);                       // evil floating point bit level hacking
		i = 0x5F375A86 - (i >> 1);               // what the fuck? 
		y = convert<float>(i);
		y = y * (threehalfs - (x2 * y * y));   // 1st iteration
		y = y * (threehalfs - (x2 * y * y));   // 2nd iteration, this can be removed
		y = y * (threehalfs - (x2 * y * y));
		return y;
	}

	double q_rsqrt(double number) {
		long long i;
		double x2, y;
		const double threehalfs = 1.5F;

		x2 = number * 0.5F;
		y = number;
		i = convert<long long>(y);                       // evil floating point bit level hacking
		i = 0x5FE6EB50C7B537A9 - (i >> 1);               // what the fuck? 
		y = convert<double>(i);
		y = y * (threehalfs - (x2 * y * y));   // 1st iteration
		y = y * (threehalfs - (x2 * y * y));   // 2nd iteration, this can be removed
		y = y * (threehalfs - (x2 * y * y));
		y = y * (threehalfs - (x2 * y * y));
		return y;
	}
}
}
