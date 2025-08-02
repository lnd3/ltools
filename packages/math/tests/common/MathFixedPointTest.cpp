#include "testing/Test.h"
#include "logging/Log.h"

#include "math/MathAll.h"

using namespace l;
using namespace l::math::fp;

TEST(MathFixedPoint, Basic) {
	{
		FixedPoint fp(100300001.12);
		TEST_TRUE(fp.scale() == 100, "");
		TEST_FUZZY(fp.toDouble(), 100300001.12, 0.001, "");
		TEST_TRUE(fp.rawValue() == 10030000112, "");
	}
	{
		FixedPoint fp(100300001000016.0);
		TEST_TRUE(fp.scale() == 1, "");
		TEST_FUZZY(fp.toDouble(), 100300001000016.0, 0.01, "");
		TEST_TRUE(fp.rawValue() == 100300001000016, "");
	}
	{
		FixedPoint fp(1000.1f, 0);
		TEST_TRUE(fp.scale() == 1, "");
		TEST_FUZZY(fp.toFloat(), 1000.0f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 1000, "");
	}
	{
		FixedPoint fp(1000.1f, 1);
		TEST_TRUE(fp.scale() == 10, "");
		TEST_FUZZY(fp.toFloat(), 1000.1f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 10001, "");
	}
	{
		FixedPoint fp(1001.1f, 1);
		TEST_TRUE(fp.scale() == 10, "");
		TEST_FUZZY(fp.toFloat(), 1001.1f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 10011, "");
	}
	{
		FixedPoint fp(0.0001f);
		TEST_TRUE(fp.scale() == 10000, "");
		TEST_FUZZY(fp.toFloat(), 0.0001f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 1, "");
	}
	{
		FixedPoint fp(0.00000004f);
		TEST_TRUE(fp.scale() == 100000000, "");
		TEST_FUZZY(fp.toFloat(), 0.00000004f, 0.00000001f, "");
		TEST_TRUE(fp.rawValue() == 4, "");
	}
	{
		FixedPoint fp(0.002f);
		TEST_TRUE(fp.scale() == 1000, "");
		TEST_FUZZY(fp.toFloat(), 0.002f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 2, "");
	}
	{
		FixedPoint fp(0.00213f);
		TEST_TRUE(fp.scale() == 100000, "");
		TEST_FUZZY(fp.toFloat(), 0.00213f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 213, "");
	}

	{
		FixedPoint fp("0.002");
		TEST_TRUE(fp.scale() == 1000, "");
		TEST_FUZZY(fp.toFloat(), 0.002f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 2, "");
	}
	{
		FixedPoint fp("0.00213");
		TEST_TRUE(fp.scale() == 100000, "");
		TEST_FUZZY(fp.toFloat(), 0.00213f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 213, "");
	}
	{
		FixedPoint fp("0.001f");
		TEST_TRUE(fp.scale() == 1000, "");
		TEST_FUZZY(fp.toFloat(), 0.001f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 1, "");
	}
	{
		FixedPoint fp("10000.0f");
		TEST_TRUE(fp.scale() == 1, "");
		TEST_FUZZY(fp.toFloat(), 10000.0f, 0.0000001f, "");
		TEST_TRUE(fp.rawValue() == 10000, "");
	}
	return 0;
}

