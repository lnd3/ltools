#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include "logging/Log.h"
#include "meta/Reflection.h"

namespace l::container {

	template<const size_t MaxElementSize>
	class VectorPolymorphic {
	public:
		static const size_t DataSize = MaxElementSize - 2;

		VectorPolymorphic(size_t reserve) {
			mContainer.reserve(reserve);
		}
		virtual ~VectorPolymorphic() = default;

		class ContainerType {
		public:
			inline void* data() {
				return (char*)this + 2;
			}

			template<class T>
			bool isType() {
				auto type = l::meta::class_checksum<T>();
				return mDataTypeCheckSum == (type % 255);
			}

			template<class T>
			void setType() {
				auto type = l::meta::class_checksum<T>();
				mDataTypeCheckSum = type % 255;
			}

			uint8_t mTypeInfo;
			uint8_t mDataTypeCheckSum;
			uint8_t mData[DataSize];
		};

		struct Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ContainerType;
			using pointer = value_type*;  // or also value_type*
			using reference = value_type&;  // or also value_type&

			Iterator(pointer ptr) : m_ptr(ptr) {}

			reference operator*() const { return *m_ptr; }
			pointer operator->() { return m_ptr; }

			template<class T>
			T* get() {
				if (!m_ptr->isType<T>()) {
					return nullptr;
				}
				return reinterpret_cast<T*>(m_ptr->data());
			}

			Iterator& operator+=(int index) { m_ptr+=index; return *this; }

			// Prefix increment
			Iterator& operator++() { m_ptr++; return *this; }

			// Postfix increment
			Iterator operator++(int) { 
				Iterator tmp = *this; ++(*this); return tmp; }

			friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
			friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

		private:
			pointer m_ptr;
		};

		Iterator begin() { return Iterator(mContainer.data()); 
		}
		Iterator end() { 
			return Iterator(mContainer.data() + mContainer.size()); 
		}

		Iterator operator[](size_t index) {
			return Iterator(mContainer.data() + index);
		}

		template<class T>
		void push_back(T&& value) {
			static_assert(sizeof(T) <= sizeof(ContainerType));
			ContainerType* ptr = mContainer.data() + mContainer.size();
			mContainer.push_back({});
			ptr->setType<T>();
			memcpy(ptr->data(), (void*)&value, sizeof(T));
		}

		void erase(size_t pos) {
			mContainer.erase(mContainer.begin() + pos);
		}

	protected:
		std::vector<ContainerType> mContainer;
	};
}
