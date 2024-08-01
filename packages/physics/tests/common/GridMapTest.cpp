#include "physics/GridMap.h"

#include "testing/Test.h"
#include "logging/LoggingAll.h"

#include <random>

using namespace l;

TEST(GridMap, Init) {
	std::vector<float> vertices = {
		0.1f, 0.1f, // 0
		0.11f, 0.11f, // 0
		0.5f, 0.1f, // 1
		0.45f, 0.51f, // 3
		0.1f, 0.51f // 2
	};

	physics::GridMap<2, float> grid(vertices);

	grid.FillGrid(0.1f);

	uint32_t count = 0;
	grid.FindPairs(0.1f, [&](uint32_t i, uint32_t j) {
		count++;
		});

	TEST_TRUE(count == 1, "");

	return 0;
}

TEST(GridMap, CheckDiagonals) {
	std::vector<float> xyz1 = {
		1.0f, 1.0f, 1.0f,
		2.0f, 2.0f, 2.0f
	};
	std::vector<float> xyz2 = {
		1.0f, 2.0f, 1.0f,
		2.0f, 1.0f, 2.0f
	};
	std::vector<float> xz1 = {
		1.0f, 1.0f, 2.0f,
		2.0f, 2.0f, 1.0f
	};
	std::vector<float> xz2 = {
		2.0f, 1.0f, 1.0f,
		1.0f, 2.0f, 2.0f
	};

	{
		physics::GridMap<3, float> grid(xyz1);
		grid.FillGrid(2.0f);
		uint32_t count = 0;
		grid.FindPairs(1.8f, [&](uint32_t i, uint32_t j) {
			count++;
			});
		TEST_TRUE(count == 1, "");
	}
	{
		physics::GridMap<3, float> grid(xyz2);
		grid.FillGrid(2.0f);
		uint32_t count = 0;
		grid.FindPairs(1.8f, [&](uint32_t i, uint32_t j) {
			count++;
			});
		TEST_TRUE(count == 1, "");
	}
	{
		physics::GridMap<3, float> grid(xz1);
		grid.FillGrid(2.0f);
		uint32_t count = 0;
		grid.FindPairs(1.8f, [&](uint32_t i, uint32_t j) {
			count++;
			});
		TEST_TRUE(count == 1, "");
	}
	{
		physics::GridMap<3, float> grid(xz2);
		grid.FillGrid(2.0f);
		uint32_t count = 0;
		grid.FindPairs(1.8f, [&](uint32_t i, uint32_t j) {
			count++;
			});
		TEST_TRUE(count == 1, "");
	}

	return 0;
}

TEST(GridMap, AllCellsChecked) {
	std::vector<float> vertices;

	for (int i = 1; i < 3; i++) {
		for (int j = 1; j < 3; j++) {
			for (int k = 1; k < 3; k++) {
				vertices.push_back(static_cast<float>(i));
				vertices.push_back(static_cast<float>(j));
				vertices.push_back(static_cast<float>(k));
			}
		}
	}

	physics::GridMap<3, float> grid(vertices);

	grid.FillGrid(2.0f);

	uint32_t count = 0;
	grid.FindPairs(1.8f, [&](uint32_t i, uint32_t j) {
		count++;
		});

	TEST_TRUE(count == 28, "");

	count = 0;
	grid.FindPairs(1.6f, [&](uint32_t i, uint32_t j) {
		count++;
		});

	TEST_TRUE(count == 24, "");

	return 0;
}

TEST(GridMap, StressTest) {
	std::vector<float> vertices;

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> dist6(-5.0f, 5.0f);

	uint32_t maxCount = 30000;
	vertices.reserve(maxCount);
	for (uint32_t i = 0; i < maxCount; i++) {
		float value = static_cast<float>(dist6(rng));
		vertices.emplace_back(value);
	}

	physics::GridMap<3, float> grid(vertices);

	grid.FillGrid(0.2f);

	size_t count = 0;
	grid.FindPairs(0.2f, [&](uint32_t i, uint32_t j) {
		count++;
		});

	LOG(LogInfo) << "Found " << count << " pairs";

	TEST_TRUE(count > 500 && count < 1100, "");
	return 0;
}
