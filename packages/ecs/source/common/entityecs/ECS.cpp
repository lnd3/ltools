
#include "ecs/entityecs/ECS.h"


namespace l::ecs
{
	Internal::BaseComponentContainer* Entity::get(const TypeIndex& typeIndex) {
		auto found = components.find(typeIndex);
		if (found != components.end())
		{
			return found->second;
		}

		return nullptr;
	}


	Entity* World::create()
	{
		++lastEntityId;
		Entity* ent = std::allocator_traits<EntityAllocator>::allocate(entAlloc, 1);
		std::allocator_traits<EntityAllocator>::construct(entAlloc, ent, this, lastEntityId);
		entities.push_back(ent);

		emit<Events::OnEntityCreated>({ ent });

		return ent;
	}

}


