#include <memory>
#include <sstream>
#include <conio.h>
#include <mutex>
#include <vector>
#include <functional>
#include <iostream>
#include <Windows.h>

#include "logging/Log.h"
#include "testing/Test.h"
#include "logging/String.h"
#include "ecs/Entity.h"

using namespace l;


TEST(Entity, Storage)
{
	{
		ecs2::Entity e;

		struct Data {
			float a = 0.0f;
			float b = 0.0f;
		};

		ecs2::CoreComponent<Data> c;

		float acopy = c->a;
		auto& d = c.data();

		e.add(&c);
	}

	return 0;
}

TEST(Entity, EntitiesMaxSize) {
	{

	}
	return 0;
}

