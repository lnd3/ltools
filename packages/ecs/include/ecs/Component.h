#pragma once

#include <vector>



namespace l::ecs2 {
	



	template<class T>
	class ComponentCache {
	public:
		ComponentCache() {

		}
		~ComponentCache() {

		}

		void add(uint32_t id) {

		}

		T& find(uint32_t id) {

		}

	protected:
		std::vector<T> mComponents;
	};


}
