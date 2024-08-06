#include <memory>
#include <sstream>
#include <mutex>
#include <vector>
#include <functional>
#include <iostream>

#include "logging/LoggingAll.h"
#include "testing/Test.h"

#include "ecs/entityecs/ECSExt.h"

using namespace l;

using namespace l::ecs;
using namespace l::logging;

struct Position {
	float x;
	float y;
};

struct Rotation {
	float angle;
};

struct SomeComponent {};

struct DeleteEvent {
	int num;
};

class TestSystem2 : public EntitySystem2,
	public EventSubscriber<DeleteEvent>
{
public:
	virtual ~TestSystem2() {}

	virtual void configure(class World* world) override
	{
		world->subscribe<DeleteEvent>(this);

		auto world2 = reinterpret_cast<World2*>(world);
		mComponents = world2->getComponentCache<Position, Rotation>();
	}

	virtual void unconfigure(class World* world) override
	{
		mComponents = nullptr;
		world->unsubscribe<DeleteEvent>(this);
	}

	virtual void tick(class World* world, ECS_TICK_TYPE tick) override
	{
		mComponents->each([&](Entity*, ComponentHandle<Position> pos, ComponentHandle<Rotation> rot) {
			pos->x += tick.deltaTime;
			pos->y += tick.deltaTime;
			rot->angle += tick.deltaTime * 2;
		});
	}

	virtual void receive(class World* world, const DeleteEvent& event) override
	{
		LOG(LogTest) << "I received SomeEvent with value " << event.num << "!";

		// Let's delete an entity while iterating because why not?
		world->all([&](Entity* ent) {
			if (ent->getEntityId() + 1 == event.num)
				world->destroy(world->getById(event.num));

			if (ent->getEntityId() == event.num)
				LOG(LogInfo) << "Woah, we shouldn't get here!";
			});
	}

protected:
	ComponentViewCache<Position, Rotation>* mComponents = nullptr;
};

TEST(EntityECS2, Sample) {
	LOG(LogInfo) << "EntityComponentSystem Test";

	auto world = World2::createWorld();

	EntitySystem* testSystem = world->createSystem<TestSystem2>();

	Entity* ent = world->create();
	auto pos = ent->assign<Position>(0.f, 0.f);
	auto rot = ent->assign<Rotation>(0.f);

	world->tick({ 0.0f, 10.f });

	world->disableSystem(testSystem);

	world->tick({ 0.0f, 10.f });

	world->enableSystem(testSystem);

	world->tick({ 0.0f, 10.f });

	LOG(LogInfo) << "After tick(10) and EnableSystem(testSystem): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")";

	int count = 0;
	LOG(LogInfo) << "Counting entities with SomeComponent...";
	// range based for loop
	world->each2<Position, Rotation>([&](Entity* ent, ComponentHandle<Position> p, ComponentHandle<Rotation> r) {
		++count;
		//LOG(LogInfo) << "Found entity #" << ent->getEntityId() << ", p{" << p->x << "," << p->y << "}" << ", r{" << r->angle << "}";
		});

	LOG(LogInfo) << count << " entities have position and rotation!";

	for (int i = 0; i < 10; ++i)
	{
		ent = world->create();
		ent->assign<SomeComponent>();
	}

	// Emitting events
	LOG(LogInfo) << "Emit 'SomeEvent' to entity id 4";
	world->emit<DeleteEvent>({ 4 });

	world->cleanup();

	TEST_EQ(world->getCount(), 10, "");

	world->destroyWorld();

	return 0;
}

TEST(EntityECS2, BugMultipleEntitiesInViewCache) {
	auto world = World2::createWorld();

	EntitySystem* testSystem = world->createSystem<TestSystem2>();

	auto ent1 = world->create();
	ent1->assign<Position>(0.f, 0.f);
	ent1->assign<Rotation>(0.f);
	bool hasComponents = ent1->has<Position, Rotation>();
	TEST_TRUE(hasComponents, "");
	hasComponents = ent1->has<Rotation, Position>();
	TEST_TRUE(hasComponents, "");

	auto ent2 = world->create();
	ent2->assign<Position>(1.f, 1.f);
	ent2->assign<Rotation>(1.f);

	world->tick({ 0.0f, 10.f });

	world->each2<Position, Rotation>([&](Entity* ent, ComponentHandle<Position> p, ComponentHandle<Rotation> r) {
		LOG(LogInfo) << "Found entity #" << ent->getEntityId() << ", p{" << p->x << "," << p->y << "}" << ", r{" << r->angle << "}";
		});

	auto cache = world->getComponentCache<Position, Rotation>();

	world->cleanup();
	world->destroyWorld();

	return 0;
}

struct Component1 {

	float x;
	float y;
};

struct Component2 {};

PERF_TEST(EntityECS2, EntityStressTest)
{
	World2* world = World2::createWorld();

	EntitySystem* testSystem = world->createSystem<TestSystem2>();

	Entity* ent = world->create();
	auto pos = ent->assign<Position>(0.f, 0.f);
	auto rot = ent->assign<Rotation>(0.f);

	{
		PERF_TIMER("EntityStressTest::Create");
		for (int i = 0; i < 10000; ++i)
		{
			ent = world->create();
			ent->assign<Position>(0.f, 0.f);
			ent->assign<Rotation>(0.f);
			ent->assign<Component1>(0.f, 0.f);
			ent->assign<Component2>();
		}
	}

	world->tick({ 0.0f, 10.f });

	int count = 0;

	{
		PERF_TIMER("EntityStressTest::IteratePartialNaive");
		world->each<Position, Rotation>([&](Entity* ent, ComponentHandle<Position> p, ComponentHandle<Rotation> r) {
			p->x += r->angle;
			p->y += r->angle;
			++count;
			});
	}
	{
		PERF_TIMER("EntityStressTest::IterateFullNaive");
		world->each<Position, Rotation, Component1, Component2>([&](Entity* ent,
			ComponentHandle<Position> p,
			ComponentHandle<Rotation> r,
			ComponentHandle<Component1> c,
			ComponentHandle<Component2>) {
				p->x += r->angle;
				p->y += r->angle;
				p->x += c->x;
				p->y += c->y;
				++count;
			});
	}
	{
		PERF_TIMER("EntityStressTest::IteratePartial");
		world->each2<Position, Rotation>([&](Entity* ent, ComponentHandle<Position> p, ComponentHandle<Rotation> r) {
			p->x += r->angle;
			p->y += r->angle;
			++count;
			});
	}
	{
		PERF_TIMER("EntityStressTest::IterateFull");
		world->each2<Position, Rotation, Component1, Component2>([&](Entity* ent, 
			ComponentHandle<Position> p, 
			ComponentHandle<Rotation> r,
			ComponentHandle<Component1> c,
			ComponentHandle<Component2>) {
				p->x += r->angle;
				p->y += r->angle;
				p->x += c->x;
				p->y += c->y;
				++count;
			});
	}

	LOG(LogInfo) << count << " entities have position and rotation!";

	// Emitting events
	LOG(LogInfo) << "Emit 'SomeEvent' to entity id 4";
	world->emit<DeleteEvent>({ 5000 });

	world->cleanup();

	TEST_EQ(world->getCount(), 10000, "");

	world->destroyWorld();

	return 0;
}
