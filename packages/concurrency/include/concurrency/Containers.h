#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <mutex>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include "logging/Log.h"
#include "meta/Reflection.h"

namespace l::container {
	template <class T>
	class ConcurrentVector {
	public:
		ConcurrentVector() {

		}
		ConcurrentVector(ConcurrentVector<T>&&) {

		}
		ConcurrentVector(const ConcurrentVector<T>&) {

		}
		~ConcurrentVector() {

		}

		struct Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = value_type * ;  // or also value_type*
			using reference = value_type & ;  // or also value_type&

			Iterator(pointer ptr) : m_ptr(ptr) {}

			reference operator*() const { return *m_ptr; }
			pointer operator->() { return m_ptr; }

			// Prefix increment
			Iterator& operator++() { m_ptr++; return *this; }

			// Postfix increment
			Iterator operator++(int) {
				Iterator tmp = *this; ++(*this); return tmp;
			}

			friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
			friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

		private:
			pointer m_ptr;
		};

		Iterator begin() {
			return Iterator(vector.data());
		}
		Iterator end() {
			return Iterator(vector.data() + vector.size());
		}

		void push_back(T&& element) {
			std::lock_guard<std::mutex> lock(mut);
			vector.push_back(element);
		}

		T&& back() {
			std::lock_guard<std::mutex> lock(mut);
			return vector.back();
		}

		T at(uint32_t i) {
			std::lock_guard<std::mutex> lock(mut);
			return vector.at(i);
		}

		size_t size() {
			std::lock_guard<std::mutex> lock(mut);
			return vector.size();
		}

	private:
		std::mutex mut;
		std::vector<T> vector;
	};

	template<class Base>
	class vector_cc {
	public:
		virtual ~vector_cc() = default;

		void push_back(std::unique_ptr<Base> value) {
			std::lock_guard<std::mutex> lock(mutex);
			vector.push_back(std::move(value));
		}

		template <class T, class = meta::IsDerived<Base, T>>
		std::unique_ptr<T> erase(size_t pos) {
			std::lock_guard<std::mutex> lock(mutex);
			auto& it = vector.at(pos);
			Base* element_ptr = it.release();
			vector.erase(vector.begin() + pos);
			return std::unique_ptr<T>(dynamic_cast<T*>(element_ptr));
		}

		template <class T, class = meta::IsDerived<Base, T>, class... Types>
		void push_back(Types&&... args) {
			auto p = std::make_unique<T>(std::forward<Types&&>(args)...);
			push_back(std::move(p));
		}

		template<class T, class = meta::IsDerived<Base, T>>
		T* loan(size_t pos) const {
			std::lock_guard<std::mutex> lock(mutex);
			return dynamic_cast<T*>(vector.at(pos).get());
		}

	protected:
		mutable std::mutex mutex;
		std::vector<std::unique_ptr<Base>> vector;
	};

	template<class K, class Base>
	class map_cc {
	public:
		virtual ~map_cc() = default;

		template<class T, class = meta::IsDerived<Base, T>>
		void set(const K& key, std::unique_ptr<T> value) {
			std::lock_guard<std::mutex> lock(mutex);
			map.emplace(key, std::move(value));
		}

		template<class T, class = meta::IsDerived<Base, T>>
		void set(const K& key, T* value) {
			std::lock_guard<std::mutex> lock(mutex);
			map.emplace(key, std::unique_ptr<T>(value));
		}

		template <class T, class = meta::IsDerived<Base, T>, class... Types>
		void make(const K& key, Types&&... args) {
			auto p = std::make_unique<T>(std::forward<Types&&>(args)...);
			set(key, std::move(p));
		}

		template <class T, class = meta::IsDerived<Base, T>>
		std::unique_ptr<T> erase(const K& key) {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = map.find(key);
			if (it != map.end()) {
				Base* element_ptr = it->second.release();
				map.erase(it);
				return std::unique_ptr<T>(dynamic_cast<T*>(element_ptr));
			}
			return nullptr;
		}


		void erase(const K& key) {
			std::lock_guard<std::mutex> lock(mutex);
			map.erase(key);
		}

		template<class T, class = meta::IsDerived<Base, T>>
		T* loan(const K& key)  const {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = map.find(key);
			if (it != map.end()) {
				return dynamic_cast<T*>(it->second.get());
			}
			return nullptr;
		}

	protected:
		mutable std::mutex mutex;
		std::map<K, std::unique_ptr<Base>> map;
	};

	template<class Base>
	class map_cc_unique {
	public:
		virtual ~map_cc_unique() {
			for (auto& it : map) {
				EXPECT(it.second.use_count() == 1) << "Shared ptr with hash " << it.first << " of type '" << meta::class_name_from_hash(it.first) << "' is still being used externally to the main storage";
			}
		};

		template<class T, class = meta::IsDerived<Base, T>>
		void set(std::shared_ptr<T> object) {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = map.find(meta::Type<T>::hash_code());
			if (it == map.end()) {
				map.emplace(meta::Type<T>::hash_code(), std::move(object));
			}
		}

		template <class T, class = meta::IsDerived<Base, T>, class... Types>
		void make(Types&&... args) {
			auto p = std::make_shared<T>(std::forward<Types&&>(args)...);
			set(std::move(p));
		}

		template<class T, class = meta::IsDerived<Base, T>>
		void erase() {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = map.find(meta::Type<T>::hash_code());
			if (it != map.end()) {
				EXPECT(it->second.use_count() == 1) << "Shared ptr with hash " << it->first << " of type '" << meta::class_name_from_hash(it->first) << "' is still being used externally to the main storage";
				map.erase(meta::Type<T>::hash_code());
			}
		}

		template<class T, class = meta::IsDerived<Base, T>>
		std::shared_ptr<T> get() const {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = map.find(meta::Type<T>::hash_code());
			if (it != map.end()) {
				return std::dynamic_pointer_cast<T>(it->second);
			}
			return {};
		}

		template<class T, class = meta::IsDerived<Base, T>>
		std::weak_ptr<T> get_weak() const {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = map.find(meta::Type<T>::hash_code());
			if (it != map.end()) {
				return std::dynamic_pointer_cast<T>(it->second);
			}
			return {};
		}

	protected:
		mutable std::mutex mutex;
		std::map<size_t, std::shared_ptr<Base>> map;
	};
}
