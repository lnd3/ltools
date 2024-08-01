#include "testing/Test.h"
#include "logging/Log.h"

using namespace l;

TEST(Testing, TestOperations) {
	ASSERT(1 == 1) << "1 is not equal to 2!";
	EXPECT(true) << "blabla";

	TEST_EQ(1, 1, "Failed to compare");
	TEST_TRUE(1 == 1, "Failed to");

	TEST_EQ(1, 1, "TEST_EQ failed");
	TEST_FALSE(false, "TEST_FALSE failed");
	TEST_TRUE(true, "TEST_TRUE failed");
	TEST_FUZZY(1.0, 1.0001, 0.001, "TEST_FUZZE failed");
	TEST_FUZZY(1.0, 0.9999, 0.001, "TEST_FUZZE failed");

	struct A {
		int i = 1;
	};
	A a;

	A* ptr = REQUIRE(&a);

	return 0;
}

int main(int, char* argw[]) {
	TEST_RUN(argw[0]);

	return 0;
}



