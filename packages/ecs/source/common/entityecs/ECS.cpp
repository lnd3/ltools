
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




}


