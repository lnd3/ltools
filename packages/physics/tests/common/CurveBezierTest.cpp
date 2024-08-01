#include "testing/Test.h"

#include "physics/CurveBezier.h"


TEST(CurveBezier, Init) {
	l::curves::Bezier b;

	b.addPoint(l::vec::Data4<double>(0.0, 0.0, 0.0, 0.0));
	b.addPoint(l::vec::Data4<double>(2.0, 0.0, 0.0, 0.0));
	b.addPoint(l::vec::Data4<double>(3.0, 0.0, 0.0, 0.0));
	b.addPoint(l::vec::Data4<double>(1.0, 0.0, 0.0, 0.0));
	b.addPoint(l::vec::Data4<double>(5.0, 0.0, 0.0, 0.0));

	auto p = b.sampleAt(0, 3);
	//TEST_TRUE(l::vec::Data4<double>(0.0, 0.0, 0.0, 0.0) == p, "");

	return 0;
}

