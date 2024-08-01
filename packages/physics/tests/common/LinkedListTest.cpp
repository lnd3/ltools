#include "physics/LinkedList.h"

#include "testing/Test.h"
#include "testing/Timer.h"

#include "logging/LoggingAll.h"

using namespace l;

TEST(LinkedList, LinkedElementsInit) {

	physics::LinkedElements<uint32_t> elements;
	elements.init(16);

	elements.add(3, 6);
	elements.add(7, 6);
	elements.add(5, 6);
	elements.add(9, 6);
	elements.add(13, 6);
	elements.add(10, 6);
	elements.add(14, 6);
	elements.add(18, 6);

	return 0;
}

PERF_TEST(LinkedList, Add) {

	physics::LinkedElements<uint32_t> elements;
	elements.init(1200000);

	{
		PERF_TIMER("LinkedList::Add");
		for (int i = 0; i < 1000000; i++) {
			elements.add(i, i);
		}
	}
	{
		PERF_TIMER("LinkedList::Get");
		for (int i = 0; i < 1000000; i++) {
			auto& e = elements.get(i, i);
			e.mData++;
		}
	}

	std::unordered_map<uint32_t, uint32_t> map;
	map.reserve(1200000);
	{
		PERF_TIMER("UnorderedMap::Add");
		for (int i = 0; i < 1000000; i++) {
			map.emplace(i, i);
		}
	}
	{
		PERF_TIMER("UnorderedMap::Get");
		for (int i = 0; i < 1000000; i++) {
			auto& e = map.at(i);
			e++;
		}
	}

	std::map<uint32_t, uint32_t> map2;
	{
		PERF_TIMER("Map::Add");
		for (int i = 0; i < 1000000; i++) {
			map2.emplace(i, i);
		}
	}
	{
		PERF_TIMER("Map::Get");
		for (int i = 0; i < 1000000; i++) {
			auto& e = map2.at(i);
			e++;
		}
	}

	return 0;
}
