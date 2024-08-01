#include "physics/BoundaryList.h"

#include "testing/Test.h"
#include "logging/LoggingAll.h"

using namespace l;

TEST(BoundaryList, SimpleSorting) {
	physics::BoundaryList list;

	list.init(2, 12);
	list.add(1, false, 1.0f);
	list.add(1, true, 3.0f);
	list.add(2, false, 2.0f);
	list.add(2, true, 4.0f);
	list.add(3, false, 3.0f);
	list.remove(4);
	list.sortInsertion(
		[](uint32_t axis, std::span<l::physics::BoundaryChange> changes) {
			TEST_TRUE_NO_RET(axis == 2, "");
			TEST_TRUE_NO_RET(changes.size() == 2, "");
			TEST_TRUE_NO_RET(changes[0].boundaryData.getOwnerId() == 1, "");
			TEST_TRUE_NO_RET(changes[0].boundaryData.isMaxBoundary(), "");
			TEST_TRUE_NO_RET(changes[0].boundaryIndex == 2, "");

			TEST_TRUE_NO_RET(changes[1].boundaryData.getOwnerId() == 2, "");
			TEST_TRUE_NO_RET(changes[1].boundaryData.isMinBoundary(), "");
			TEST_TRUE_NO_RET(changes[1].boundaryIndex == 1, "");
		},
		[](uint32_t id0, uint32_t id1, bool max) {
			TEST_TRUE_NO_RET(id0 == 1, "");
			TEST_TRUE_NO_RET(id1 == 2, "");
		});

	return 0;
}

TEST(BoundaryList, MultiSorting) {
	physics::BoundaryList list;

	list.init(2, 12);
	list.add(1, false, 1.0f);
	list.add(1, true, 3.0f);
	list.add(2, false, 2.0f);
	list.add(2, true, 4.0f);
	list.add(3, false, 1.0f);
	list.add(3, true, 3.0f);
	list.sortInsertion(
		[](uint32_t axis, std::span<l::physics::BoundaryChange> changes) {
			TEST_TRUE_NO_RET(changes.size() == 4, "");
		},
		[](uint32_t id0, uint32_t id1, bool max) {
			TEST_TRUE_NO_RET((id0 == 1 && id1 == 2) || (id0 == 2 && id1 == 3) || (id0 == 1 && id1 == 3), "");
		});

	return 0;
}
