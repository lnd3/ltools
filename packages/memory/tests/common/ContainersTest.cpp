#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"
#include "memory/Containers.h"

TEST(Containers, VectorOps) {
	{
		std::vector<int> v = { 1, 3, 5, 32, 53, 2, 95 };
		auto v_sub3 = l::container::vector_extract(v, 4u, 3u);
		auto v_sub2 = l::container::vector_extract(v, 2u, 2u);
		auto v_sub1 = l::container::vector_extract(v, 0u, 2);

		TEST_TRUE(v.empty(), "");
		TEST_TRUE(v_sub3.size() == 3u, "");
		TEST_TRUE(v_sub2.size() == 2u, "");
		TEST_TRUE(v_sub1.size() == 2u, "");
		TEST_TRUE(v_sub3[0] == 53u, "");
		TEST_TRUE(v_sub2[0] == 5u, "");
		TEST_TRUE(v_sub1[0] == 1u, "");
		TEST_TRUE(v_sub3[2] == 95u, "");
		TEST_TRUE(v_sub2[1] == 32u, "");
		TEST_TRUE(v_sub1[1] == 3u, "");
	}
	{
		std::vector<int> v = { 1, 3, 5, 32, 53, 2, 95 };
		auto v_sub1 = l::container::vector_extract(v, 0u, 2u);
		auto v_sub2 = l::container::vector_extract(v, 0u, 2u);
		auto v_sub3 = l::container::vector_extract(v, 0u, 3u);

		TEST_TRUE(v.empty(), "");
		TEST_TRUE(v_sub3.size() == 3u, "");
		TEST_TRUE(v_sub2.size() == 2u, "");
		TEST_TRUE(v_sub1.size() == 2u, "");
		TEST_TRUE(v_sub3[0] == 53u, "");
		TEST_TRUE(v_sub2[0] == 5u, "");
		TEST_TRUE(v_sub1[0] == 1u, "");
		TEST_TRUE(v_sub3[2] == 95u, "");
		TEST_TRUE(v_sub2[1] == 32u, "");
		TEST_TRUE(v_sub1[1] == 3u, "");
	}

	return 0;
}




