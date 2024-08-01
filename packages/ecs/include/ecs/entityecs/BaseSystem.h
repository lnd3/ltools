#pragma once

#include <memory>
#include <sstream>
#include <conio.h>
#include <mutex>
#include <vector>
#include <functional>
#include <iostream>
#include <Windows.h>

#include "logging/LoggingAll.h"

#include "ecs/entityecs/ECSExt.h"

namespace l::ecs {

template<class ...Types>
class BaseSystem : public EntitySystem2,
	public EventSubscriber<Events::OnEntityCreated>,
	public EventSubscriber<Events::OnEntityDestroyed>
{
public:
	virtual ~BaseSystem() {}

	virtual void configure(class World* world) {
		EntitySystem2::configure(world);

		world->subscribe<Events::OnEntityCreated>(this);
		world->subscribe<Events::OnEntityDestroyed>(this);
		
		auto world2 = reinterpret_cast<World2*>(world);
		mComponents = world2->getComponentCache<Types...>();
	}

	virtual void unconfigure(class World* world) {
		EntitySystem2::unconfigure(world);
	}

	virtual void receive(class World*, const Events::OnEntityCreated& event) {
		LOG(LogTest) << "An entity was created! " << event.entity->getEntityId();
	}

	virtual void receive(class World*, const Events::OnEntityDestroyed& event) {
		LOG(LogTest) << "An entity was destroyed! " << event.entity->getEntityId();
	}

	auto& getComponents() {
		return mComponents->getComponents();
	}

protected:
	ComponentViewCache<Types...>* mComponents = nullptr;
};



template<class Type>
struct Position2
{
	Type x;
	Type y;
};

template<class Type>
struct Position3
{
	Type x;
	Type y;
	Type z;
};

template<class Type>
struct Position4
{
	Type x;
	Type y;
	Type z;
	Type w;
};

struct TouchData
{
	float x;
	float y;
	bool absolute;
};


class TouchSystem : public l::ecs::BaseSystem<TouchData> {
public:
	TouchSystem() = default;
	~TouchSystem() = default;

	void SetTouchCB(std::function<void(float, float)> mouseSet, std::function<std::tuple<float, float>()> mouseGet, std::function<std::tuple<float, float>()> screenSizeGet) {
		mMouseGet = std::move(mouseGet);
		mMouseSet = std::move(mouseSet);
		mGetScreenSize = std::move(screenSizeGet);
	}
	
	void tick(class World*, float) {
		if (!mMouseGet) {
			return;
		}

		auto [x, y] = mMouseGet();
		auto [w, h] = mGetScreenSize();

		for (auto& it : getComponents()) {
			auto& touchData = std::get<1>(it).get();

			if (touchData.absolute) {
				touchData.x = x;
				touchData.y = y;
			}
			else {
				touchData.x = w / 2.0f - x;
				touchData.y = h / 2.0f - y;
				mMouseSet(w / 2, h / 2);
			}
		}
	}

private:
	std::function<std::tuple<float, float>()> mMouseGet;
	std::function<void(float, float)> mMouseSet;
	std::function<std::tuple<float, float>()> mGetScreenSize;
};

}