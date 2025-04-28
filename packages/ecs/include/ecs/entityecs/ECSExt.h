#pragma once

#include <functional>
#include <tuple>
#include <typeinfo>
#include <vector>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iostream>

struct TickData {
	double elapsedTime;
	float deltaTime;
};

#ifndef ECS_TICK_DATA
#define ECS_TICK_TYPE TickData
#endif

template<class T>
class DefaultAllocator : public std::allocator<T> {
public:
	DefaultAllocator() noexcept {}
	DefaultAllocator(const DefaultAllocator& other) noexcept : std::allocator<T>(other) {}
	template<class U>
	DefaultAllocator(const DefaultAllocator<U>& other) noexcept : std::allocator<T>(other) {}
	~DefaultAllocator() {}

	void deallocate(T* const ptr, const size_t count) {
		std::allocator<T>::deallocate(ptr, count);
	}

	T* allocate(const size_t count) {
		return std::allocator<T>::allocate(count);
	}
};

#define ECS_ALLOCATOR_TYPE DefaultAllocator<l::ecs::Entity>

#include "ECS.h"
#include "logging/LoggingAll.h"

namespace l::ecs {

	template<class ...Types>
	size_t constexpr types_to_hash() {
		const size_t size = sizeof...(Types);
		const size_t typeIndexes[size] = { (typeid(Types).hash_code())...};

		size_t hash = 0;
		size_t i = 0;
		for (; i < size; i++) {
			hash += typeIndexes[i];
		}
		return hash;
	}

	template<class ...Types>
	std::string types_to_string() {
		const size_t size = sizeof...(Types);
		const TypeIndex typeIndexes[size] = { getTypeIndex<Types>()... };

		std::ostringstream out;
		out << typeIndexes[0].name();
		for (int i = 1; i < size; i++) {
			out << ", " << typeIndexes[i].name();
		}
		return out.str();
	}

	template<class ...Types>
	class ComponentViewCache;

	class BaseComponentViewCache :
		public EventSubscriber<Events::OnComponentAssigned2>,
		public EventSubscriber<Events::OnComponentRemoved2>
	{
	public:
		virtual ~BaseComponentViewCache() {}

		template<class ...Types>
		ComponentViewCache<Types...>& operator->() {
			return &reinterpret_cast<ComponentViewCache<Types...>*>(this);
		}

		template<class ...Types>
		ComponentViewCache<Types...>& get() {
			return *reinterpret_cast<ComponentViewCache<Types...>*>(this);
		}

		virtual void configure(World*) {};
		virtual void unconfigure(World*) {};
		virtual void receive(World*, const Events::OnComponentAssigned2&) {};
		virtual void receive(World*, const Events::OnComponentRemoved2&) {};
	};

	template<class ...Types>
	using viewType = std::function<void(Entity*, ComponentHandle<Types>...)>;

	template<class ...Types>
	class ComponentViewCache : public BaseComponentViewCache {
	public:

		ComponentViewCache() {}
		virtual ~ComponentViewCache() {
			mComponents.clear();
			mComponentMap.clear();
		}

		void each(viewType<Types...> viewFunc) {
			for (auto& handle : mComponents) {
				std::apply(viewFunc, handle);
			}
		}

		Entity* getFirst() {
			if (mComponents.empty()) {
				return nullptr;
			}
			return std::get<0>(mComponents.at(0));
		}

		auto& getComponents() {
			return mComponents;
		}

		bool tryAdd(Entity* e) {
			if (e->has<Types...>() && !mComponentMap.contains(e)) {
				mComponents.emplace_back(e, e->template get<Types>()...);
				mComponentMap.emplace(e, std::tuple{ e->template get<Types>()... });
				return true;
			}
			return false;
		}

		bool tryRemove(Entity* e) {
			if (e->has<Types...>() && mComponentMap.contains(e)) {
				auto it = std::find_if(mComponents.begin(), mComponents.end(), [&](const auto& entry) -> bool {
					if (std::get<0>(entry) == e) {
						return true;
					}
					return false;
					});

				if (it != mComponents.end()) {
					mComponents.erase(it);
				}
				mComponentMap.erase(e);
				return true;
			}
			return false;
		}

		bool has(TypeIndex index) {
			const size_t size = sizeof...(Types);
			const TypeIndex typeIndexes[size] = { getTypeIndex<Types>()... };

			for (size_t i = 0; i < size; i++) {
				if (index == typeIndexes[i]) {
					return true;
				}
			}
			return false;
		}

		std::string types() {
			return types_to_string<Types...>();
		}

		virtual void configure(World* world) {
			world->subscribe<Events::OnComponentAssigned2>(this);
			world->subscribe<Events::OnComponentRemoved2>(this);
		}

		virtual void unconfigure(World* world) {
			world->unsubscribe<Events::OnComponentAssigned2>(this);
			world->unsubscribe<Events::OnComponentRemoved2>(this);
		}

		virtual void receive(World*, const Events::OnComponentAssigned2& event) {
			if (tryAdd(event.entity)) {
				for (auto& listener : mEventListenersAssign) {
					listener.second(event);
				}
			}
		}

		virtual void receive(World*, const Events::OnComponentRemoved2& event) {
			if (tryRemove(event.entity)) {
				for (auto& listener : mEventListenersRemove) {
					listener.second(event);
				}
			}
		}

		template<class T>
		int32_t registerListener(std::function<void(const T&)> listener) {
			if constexpr (std::is_same_v<T, Events::OnComponentAssigned2>){
				mEventListenersAssign.push_back({ mIdEnumerator, listener });
			}
			else if constexpr (std::is_same_v<T, Events::OnComponentRemoved2>){
				mEventListenersRemove.push_back({ mIdEnumerator, listener });
			}
			else {
				return -1;
			}
			return mIdEnumerator++;
		}

		template<class T>
		void unregisterListener(int32_t id) {
			if constexpr (std::is_same_v<T, Events::OnComponentAssigned2>){
				std::erase_if(mEventListenersAssign, [cid = id](auto& listener) {
					if (listener.first == cid) {
						return true;
					}
					return false;
					});
			}
			else if constexpr (std::is_same_v<T, Events::OnComponentRemoved2>){
				std::erase_if(mEventListenersRemove, [cid = id](auto& listener) {
					if (listener.first == cid) {
						return true;
					}
					return false;
					});
			}
		}
	protected:

		std::vector<std::tuple<Entity*, ComponentHandle<Types>...>> mComponents;
		std::map<Entity*, std::tuple<ComponentHandle<Types>...>> mComponentMap;

		int32_t mIdEnumerator = 0;
		std::vector<std::pair<int32_t, std::function<void(const Events::OnComponentAssigned2&)>>> mEventListenersAssign;
		std::vector<std::pair<int32_t, std::function<void(const Events::OnComponentRemoved2&)>>> mEventListenersRemove;
	};

	class EntitySystem2 : public l::ecs::EntitySystem
	{
	public:
		virtual ~EntitySystem2() {}

		virtual void configure(World*)
		{
		}

		virtual void unconfigure(World* world)
		{
			world->unsubscribeAll(this);
		}

#ifdef ECS_TICK_TYPE_VOID
		virtual void tick(World* world)
#else
		virtual void tick(World*, ECS_TICK_TYPE) override
#endif
		{

		}
	};

	class World2 : public l::ecs::World {
	public:
		using WorldAllocator2 = std::allocator_traits<Allocator>::template rebind_alloc<World2>;

		World2(Allocator alloc) : World(alloc) {}

		virtual ~World2() {
			for (auto& cache : mComponentCacheMap) {
				cache.second->unconfigure(this);
				cache.second.reset();
			}
			mComponentCacheMap.clear();
		}

		/**
		* Use this function to construct the world with a custom allocator.
		*/
		static World2* createWorld(Allocator alloc)
		{
			WorldAllocator2 worldAlloc(alloc);
			World2* world = std::allocator_traits<WorldAllocator2>::allocate(worldAlloc, 1);
			std::allocator_traits<WorldAllocator2>::construct(worldAlloc, world, alloc);
			
			return world;
		}

		/**
		* Use this function to construct the world with the default allocator.
		*/
		static World2* createWorld()
		{
			return createWorld(Allocator());
		}

		EntitySystem* registerSystem(EntitySystem*) {
			EXPECT(false) << "Obsolete function. Please use 'createSystem'.";
			return nullptr;
		}

		template<class T, class ...Params>
		EntitySystem* createSystem(Params&&... params)
		{
			T* system = new T(std::forward<Params&&>(params)...);

			systems.push_back(system);
			system->configure(this);

			return system;
		}

		// Use this to destroy the world instead of calling delete.
		// This will emit OnEntityDestroyed events and call EntitySystem::unconfigure as appropriate.
		void destroyWorld()
		{
			WorldAllocator2 alloc(entAlloc);
			std::allocator_traits<WorldAllocator2>::destroy(alloc, this);
			std::allocator_traits<WorldAllocator2>::deallocate(alloc, this, 1);
		}

		template<typename... Types>
		ComponentViewCache<Types...>* getComponentCache() {
			const size_t type = types_to_hash<Types...>();
			auto it = mComponentCacheMap.find(type);
			if (it == mComponentCacheMap.end()) {
				mComponentCacheMap.emplace(type, std::make_unique<ComponentViewCache<Types...>>());

				it = mComponentCacheMap.find(type);
				it->second->configure(this);

				for (auto e : entities) {
					if (e->has<Types...>()) {
						const size_t size = sizeof...(Types);
						for (size_t i = 0; i < size; i++) {
							// No access to T so won't work
							//const size_t type = types_to_hash<Types...>();
							//auto container = reinterpret_cast<Internal::ComponentContainer<T>*>(e->get(typeIndexes[i]));
							//auto handle = ComponentHandle<T>(&container->data);
							//emit<Events::OnComponentAssigned<T>>({ e, handle });
							const TypeIndex typeIndexes[size] = { getTypeIndex<Types>()... };
							emit<Events::OnComponentAssigned2>({ e, typeIndexes[i], e->get(typeIndexes[i]) });
						}
					}
				}
			}
			if (!it->second) {
				LOG(LogError) << "ComponeneViewCache has map entry, but is missing the actual cache for types: " << types_to_string<Types...>();
				mComponentCacheMap.erase(it);
				return nullptr;
			}

			return reinterpret_cast<ComponentViewCache<Types...>*>(it->second.get());
		}

		template<class T>
		ComponentHandle<T> getFirst() {
			ComponentViewCache<T>* components = getComponentCache<T>();
			Entity* entity = components->getFirst();
			if (entity) {
				return entity->get<T>();
			}
			return ComponentHandle<T>();
		}

		template<typename... Types>
		void removeComponentCache() {
			auto it = mComponentCacheMap.find(types_to_hash<Types...>());
			if (it != mComponentCacheMap.end()) {
				it->second->unconfigure(this);
				mComponentCacheMap.erase(it);
			}
		}

		// TODO: whenever looping over a set of components, use a list of component caches for iteration 

		template<typename... Types>
		void each2(typename std::common_type<std::function<void(Entity*, ComponentHandle<Types>...)>>::type viewFunc)
		{
			ComponentViewCache<Types...>* cache = getComponentCache<Types...>();
			if (cache != nullptr) {
				cache->each(viewFunc);
			}
		}

	protected:
		std::map<size_t, std::unique_ptr<BaseComponentViewCache>> mComponentCacheMap;
	};




}