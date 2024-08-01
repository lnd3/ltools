#pragma once

#include <numeric>
#include <map>

#include "tools/algorithm/kdtree/KDTree.h"

namespace l {
namespace ecs2 {
	// Spatially sorted entities and components (quad tree?) based on 
	// three space dimensions and one extra dimension for abstract 
	// entities and components

	/*
	template<class T>
	class ListIndex {
	public:
		ListIndex(const T& listInstance) : mIndex() {}
		~ListIndex(){}

		size_t get() {
			return mIndex;
		}
	private:
		size_t mIndex;
	};

	template<class T>
	class ListBase {
	public:
		ListBase(const T& base) : mBasePtr(&base) {}
		~ListBase() {}

		T* operator->(const ListIndex& index) {
			return mBasePtr + index.get();
		}
	private:
		T* mBasePtr;
	};
	*/

	template<class T>
	class LinkedList {
	public:
		LinkedList(){}
		~LinkedList() {}
		T* prev;
		T* next;


	};

	class Component {
	public:
		Component() {}

		virtual ~Component() {};

		Component* mNext = nullptr;
	};

	template<class T>
	class CoreComponent : public Component {
	public:
		CoreComponent() : mData({}) {}
		CoreComponent(T&& d) : mData(std::move(d)) {}

		virtual ~CoreComponent() {};

		T* operator->() {
			return &mData;
		}

		T& data() {
			return mData;
		}
	private:
		T mData;
	};

	class Entity {
	public:
		Entity() : mId(0), mComponents(nullptr) {}
		~Entity() {}

		template<class T>
		void add(CoreComponent<T>* component) {
			if (mComponents == nullptr) {
				mComponents = component;
			}
			else {
				component->mNext = mComponents->mNext;
				mComponents = component;
			}
		}
	private:
		uint32_t mId;
		Component *mComponents;
	};



}
}
