#include "physics/BoxSweep.h"

#include "testing/Test.h"
#include "logging/LoggingAll.h"

using namespace l;

TEST(BoxSweep, Init) {
	physics::BoxSweep sweeper;

	l::vec::Data4<float> pos(3.0f, 3.0f, 3.0f, 1.0f);

	sweeper.init(12);
	auto id0 = sweeper.add(pos, 0.0f);
	auto id1 = sweeper.add(pos, 0.0f);
	auto id2 = sweeper.add(pos, 0.0f);
	
	sweeper.remove(id1);

	auto id3 = sweeper.add(pos, 0.0f);

	l::vec::Data4<float> p0(3.0f, 3.0f, 3.0f, 1.0f);
	l::vec::Data4<float> p2(6.0f, 3.0f, 3.0f, 1.0f);
	l::vec::Data4<float> p3(4.0f, 6.0f, 2.0f, 1.0f);
	sweeper.updateBox(id0, p0, 1.0f);
	sweeper.updateBox(id2, p2, 1.0f);
	sweeper.updateBox(id3, p3, 1.0f);

	sweeper.update();

	p2.x1 -= 2.0f;
	sweeper.updateBox(id2, p2, 1.0f);
	p3.x2 -= 2.0f;
	sweeper.updateBox(id3, p3, 1.0f);

	l::vec::Data4<float> p4(4.0f, 4.0f, 6.0f, 1.0f);
	auto id4 = sweeper.add(p4, 2.0f);

	sweeper.update();

	sweeper.overlapAction([](uint32_t id0, uint32_t id1) {
		LOG(LogInfo) << "id0:" << id0 << ", id1:" << id1;
		});
	return 0;
}
