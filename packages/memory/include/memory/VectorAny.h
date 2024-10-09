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

	template<size_t DataSize>
	class ContainerTypeBase {
	public:

		ContainerTypeBase() = default;
		~ContainerTypeBase() = default;

		void* data() {
			return (char*)this + sizeof(uint8_t) * 2;
		}

		template<class T>
		bool isType() {
			return mDataTypeCheckSum == l::meta::class_checksum8<T>();
		}

		template<class T>
		void setType() {
			mDataTypeCheckSum = l::meta::class_checksum8<T>();
		}

		uint8_t mTypeInfo;
		uint8_t mDataTypeCheckSum;
		uint8_t mData[DataSize];
	};

	template<size_t DataSize>
	class IteratorBase {
	public:
		using ContainerType = ContainerTypeBase<DataSize>;

		IteratorBase(ContainerType* ptr) : m_ptr(ptr) {}
		~IteratorBase() = default;

		ContainerType& operator*() const { return *m_ptr; }
		ContainerType* operator->() { return m_ptr; }

		template<class T>
		T* get() {
			if (m_ptr->mDataTypeCheckSum != l::meta::class_checksum8<T>()) {
				return nullptr;
			}
			return reinterpret_cast<T*>(m_ptr->data());
		}

		IteratorBase& operator+=(int index) { m_ptr += index; return *this; }

		// Prefix increment
		IteratorBase& operator++() { m_ptr++; return *this; }

		// Postfix increment
		IteratorBase operator++(int) {
			IteratorBase tmp = *this; ++(*this); return tmp;
		}

		friend bool operator== (const IteratorBase& a, const IteratorBase& b) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= (const IteratorBase& a, const IteratorBase& b) { return a.m_ptr != b.m_ptr; };

	private:
		ContainerType* m_ptr;
	};

	template<const size_t DataSize>
	class VectorAny {
	public:
		static const size_t ElementSize = DataSize + 2;

		using ContainerType = ContainerTypeBase<DataSize>;
		using Iterator = IteratorBase<DataSize>;

		VectorAny(size_t reserve) {
			mContainer.reserve(reserve);
		}
		virtual ~VectorAny() = default;

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
			ptr->mDataTypeCheckSum = l::meta::class_checksum8<T>();
			memcpy(ptr->data(), (void*)&value, sizeof(T));
		}

		void erase(size_t pos) {
			mContainer.erase(mContainer.begin() + pos);
		}

	protected:
		std::vector<ContainerType> mContainer;
	};
}
