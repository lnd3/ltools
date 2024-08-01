#include "physics/VecX.h"

#include "testing/Test.h"
#include "logging/LoggingAll.h"

#define EPSILON 0.0000001

using namespace l;

TEST(Vector, Data2) {
	{
		l::vec::Data2<double> x(5.3, 4.4);
		LOG(LogInfo) << x.to_string();

		TEST_FUZZY(x.floor(1).x1, 5.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(1).x2, 4.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(1).x1, 0.3, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(1).x2, 0.4, EPSILON, "floor failed");

		TEST_FUZZY(x.floor(3).x1, 1.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(3).x2, 1.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(3).x1, 2.3 / 3.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(3).x2, 1.4 / 3.0, EPSILON, "floor failed");

		TEST_FUZZY(x.floor(4.9).x1, 1.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(4.9).x2, 0.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(4.9).x1, 0.4 / 4.9, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(4.9).x2, 4.4 / 4.9, EPSILON, "floor failed");
	}
	{
		l::vec::Data2<double> x(-5.3, -4.4);
		LOG(LogInfo) << x.to_string();

		TEST_FUZZY(x.floor(1).x1, -6.0, 0.001, "floor failed");
		TEST_FUZZY(x.floor(1).x2, -5.0, 0.001, "floor failed");
		TEST_FUZZY(x.fract(1).x1, 0.7, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(1).x2, 0.6, EPSILON, "floor failed");

		TEST_FUZZY(x.floor(3).x1, -2.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(3).x2, -2.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(3).x1, 0.7 / 3.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(3).x2, 1.6 / 3.0, EPSILON, "floor failed");
	}

	return 0;
}

TEST(Vector, Data4) {
	{
		l::vec::Data4<double> x(5.3, 4.4, 5.3, 4.4);
		LOG(LogInfo) << x.to_string();

		TEST_FUZZY(x.floor(1).x1, 5.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(1).x2, 4.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(1).x1, 0.3, EPSILON, "fract failed");
		TEST_FUZZY(x.fract(1).x2, 0.4, EPSILON, "fract failed");

		TEST_FUZZY(x.floor(3).x1, 1.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(3).x2, 1.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(3).x1, 2.3 / 3.0, EPSILON, "fract failed");
		TEST_FUZZY(x.fract(3).x2, 1.4 / 3.0, EPSILON, "fract failed");

		TEST_FUZZY(x.floor(4.9).x1, 1.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(4.9).x2, 0.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(4.9).x1, 0.4 / 4.9, EPSILON, "fract failed");
		TEST_FUZZY(x.fract(4.9).x2, 4.4 / 4.9, EPSILON, "fract failed");
	}
	{
		l::vec::Data4<double> x(-5.3, -4.4, -5.3, -4.4);
		LOG(LogInfo) << x.to_string();

		TEST_FUZZY(x.floor(1).x1, -6.0, 0.001, "floor failed");
		TEST_FUZZY(x.floor(1).x2, -5.0, 0.001, "floor failed");
		TEST_FUZZY(x.fract(1).x1, 0.7, EPSILON, "fract failed");
		TEST_FUZZY(x.fract(1).x2, 0.6, EPSILON, "fract failed");

		TEST_FUZZY(x.floor(3).x1, -2.0, EPSILON, "floor failed");
		TEST_FUZZY(x.floor(3).x2, -2.0, EPSILON, "floor failed");
		TEST_FUZZY(x.fract(3).x1, 0.7 / 3.0, EPSILON, "fract failed");
		TEST_FUZZY(x.fract(3).x2, 1.6 / 3.0, EPSILON, "fract failed");
	}

	return 0;
}

