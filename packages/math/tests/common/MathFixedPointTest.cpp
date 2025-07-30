#include "testing/Test.h"
#include "logging/Log.h"

#include "math/MathAll.h"

using namespace l;
using namespace l::math::fp;

TEST(MathFixedPoint, Basic) {
	{
		FixedPoint fp(0.0001f, 1);
		TEST_TRUE(fp.scale() == 10000, "");
		TEST_FUZZY(fp.toFloat(), 0.0001f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 1, "");
	}
	{
		FixedPoint fp("0.002", 1);
		TEST_TRUE(fp.scale() == 1000, "");
		TEST_FUZZY(fp.toFloat(), 0.002f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 2, "");
	}
	{
		FixedPoint fp("0.00213", 3);
		TEST_TRUE(fp.scale() == 100000, "");
		TEST_FUZZY(fp.toFloat(), 0.00213f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 213, "");
	}
	{
		FixedPoint fp(0.002f, 1);
		TEST_TRUE(fp.scale() == 1000, "");
		TEST_FUZZY(fp.toFloat(), 0.002f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 2, "");
	}
	{
		FixedPoint fp(0.00213f, 3);
		TEST_TRUE(fp.scale() == 100000, "");
		TEST_FUZZY(fp.toFloat(), 0.00213f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 213, "");
	}

	return 0;
}

