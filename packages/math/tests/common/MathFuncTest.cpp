#include "testing/Test.h"

#include "math/MathAll.h"

using namespace l;

TEST(MathFunc, Cbrt) {
	TEST_FUZZY(math::cbrt(27.0), 3.0, 1e-12, "");
	TEST_FUZZY(math::cbrt(-8.0), -2.0, 1e-12, "");
	TEST_FUZZY(math::cbrt(0.0), 0.0, 1e-12, "");
	TEST_FUZZY(math::cbrt(2.0) * math::cbrt(2.0) * math::cbrt(2.0), 2.0, 1e-12, "");
	TEST_FUZZY(math::cbrt(64.0f), 4.0f, 1e-5f, "");
	TEST_FUZZY(math::cbrt(1e-9), 1e-3, 1e-15, "");
	return 0;
}
