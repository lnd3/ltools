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

ECS_TYPE_IMPLEMENTATION;

using namespace l::ecs;
using namespace l::logging;

struct Position
{
	ECS_DECLARE_TYPE;

	Position(float x, float y) : x(x), y(y) {}
	Position() : x(0.0f), y(0.0f) {}

	float x;
	float y;
};

ECS_DEFINE_TYPE(Position);

struct Rotation
{
	ECS_DECLARE_TYPE;

	Rotation(float angle) : angle(angle) {}
	Rotation() : angle(0.0f){ }

	float angle;
};

ECS_DEFINE_TYPE(Rotation);

struct SomeComponent
{
	ECS_DECLARE_TYPE;

	SomeComponent() {}
};

ECS_DEFINE_TYPE(SomeComponent);

struct DeleteEvent
{
	ECS_DECLARE_TYPE;

	int num;
};

ECS_DEFINE_TYPE(DeleteEvent);

class TestSystem : public EntitySystem2,
	public EventSubscriber<Events::OnComponentRemoved<Position>>,
	public EventSubscriber<Events::OnComponentRemoved<Rotation>>,
	public EventSubscriber<DeleteEvent>
{
public:
	virtual ~TestSystem() {}

	virtual void configure(class World* world) override
	{
		world->subscribe<Events::OnComponentRemoved<Position>>(this);
		world->subscribe<Events::OnComponentRemoved<Rotation>>(this);
		world->subscribe<DeleteEvent>(this);

		auto world2 = reinterpret_cast<World2*>(world);
		components = world2->getComponentCache<Position, Rotation>();
	}

	virtual void unconfigure(class World* world) override
	{
		components = nullptr;

		world->unsubscribe<DeleteEvent>(this);
		world->unsubscribe<Events::OnComponentRemoved<Rotation>>(this);
		world->unsubscribe<Events::OnComponentRemoved<Position>>(this);
	}

	virtual void tick(class World* world, ECS_TICK_TYPE tick) override
	{
		components->each([&](Entity*, ComponentHandle<Position> pos, ComponentHandle<Rotation> rot) {
			pos->x += tick.deltaTime;
			pos->y += tick.deltaTime;
			rot->angle += tick.deltaTime * 2;
		});
	}

	virtual void receive(class World* world, const Events::OnComponentRemoved<Position>& event) override
	{
		LOG(LogTest) << "A position component was removed!";
	}

	virtual void receive(class World* world, const Events::OnComponentRemoved<Rotation>& event) override
	{
		LOG(LogTest) << "A rotation component was removed! ";
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

private:
	ComponentViewCache<Position, Rotation>* components;
};


TEST(EntityECS, ComponentCache)
{
	auto world = World2::createWorld();
	auto cache = world->getComponentCache<Position, Rotation>();
	world->removeComponentCache<Position, Rotation>();

	world->destroyWorld();

	return 0;
}

TEST(EntityECS, LifeCycle)
{
	LOG(LogInfo) << "EntityComponentSystem Test";

	auto world = World2::createWorld();

	EntitySystem* entitySystem = world->createSystem<TestSystem>();

	Entity* ent = world->create();
	auto pos = ent->assign<Position>(0.f, 0.f);
	auto rot = ent->assign<Rotation>(0.f);

	world->tick({ 0.0f, 10.f });

	world->cleanup();

	world->destroyWorld();
	
	return 0;
}

TEST(EntityECS, Sample)
{
	LOG(LogInfo) << "EntityComponentSystem Test";

	auto world = World2::createWorld();

	EntitySystem* entitySystem = world->createSystem<TestSystem>();

	Entity* ent = world->create();
	auto pos = ent->assign<Position>(0.f, 0.f);
	auto rot = ent->assign<Rotation>(0.f);

	LOG(LogInfo) << "size of an entity: " << sizeof(Entity);
	LOG(LogInfo) << "size of Position: " << sizeof(Position);
	LOG(LogInfo) << "size of Rotation: " << sizeof(Rotation);

	LOG(LogInfo) << "Initial values: position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")";

	world->tick({ 0.0f, 10.f });

	LOG(LogInfo) << "After tick(10): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")";

	world->disableSystem(entitySystem);

	world->tick({ 0.0f, 10.f });

	LOG(LogInfo) << "After tick(10) and DisableSystem(testSystem): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")";

	world->enableSystem(entitySystem);

	world->tick({ 0.0f, 10.f });

	TEST_FUZZY2(pos->x, 20.0f, "");
	TEST_FUZZY2(pos->y, 20.0f, "");
	TEST_FUZZY2(rot->angle, 40.0f, "");

	LOG(LogInfo) << "After tick(10) and EnableSystem(testSystem): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")";

	ent->remove<Position>();
	ent->remove<Rotation>();

	LOG(LogInfo) << "Creating more entities...";

	for (int i = 0; i < 10; ++i)
	{
		ent = world->create();
		ent->assign<SomeComponent>();
	}

	int count = 0;
	LOG(LogInfo) << "Counting entities with SomeComponent...";
	// range based for loop
	for (auto ent : world->each<SomeComponent>())
	{
		++count;
		LOG(LogInfo) << "Found entity #" << ent->getEntityId();
	}
	LOG(LogInfo) << count << " entities have SomeComponent!";

	// Emitting events
	world->emit<DeleteEvent>({ 4 });

	TEST_EQ(world->getCount(), 11, "");

	LOG(LogInfo) << "We have " << world->getCount() << " entities right now.";
	world->cleanup();

	TEST_EQ(world->getCount(), 10, "");

	LOG(LogInfo) << "After a cleanup, we have " << world->getCount() << " entities.";

	LOG(LogInfo) << "Destroying the world...";

	world->destroyWorld();

	return 0;
}
