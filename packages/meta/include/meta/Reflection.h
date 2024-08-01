#pragma once

#include <string>
#include <unordered_map>

#include "logging/Macro.h"
#include "logging/String.h"

#include "Template.h"

#include <mutex>

namespace l {
namespace meta {

	std::string_view class_name_from_hash(size_t hash_code);

	template<class T>
	std::string_view class_raw_name() {
		return typeid(T).raw_name();
	}

	template<class T>
	std::string_view class_name() {
		return typeid(T).name();
	}

	template<class T>
	std::string_view class_type() {
		auto name = std::string_view(typeid(T).name());
		auto parts = string::split(name);
		return parts.at(0);
	}

	template<class T>
	std::string_view class_identifier() {
		auto name = std::string_view(typeid(T).name());
		auto parts = string::split(name, " \n:`'");
		return parts.at(parts.size() - 1);
	}

	template<class T>
	size_t class_hash() {
		return typeid(T).hash_code();
	}

	template<class T>
	uint32_t class_checksum() {
		return static_cast<uint32_t>(class_hash<T>());
	}

	template<class T>
	uint16_t class_checksum16() {
		return static_cast<uint16_t>(class_hash<T>() % 65531u);
	}

	template<class T>
	uint8_t class_checksum8() {
		return static_cast<uint8_t>(class_hash<T>() % 251u);
	}

	template<class T>
	class Base {
	public:
		Base() {
			std::lock_guard<std::mutex> lock(gMutex);
			auto name = class_name_from_hash<T>();
		}

	protected:
		static std::mutex gMutex;

		template<class = IsDerived<Base<T>, T>>
		static std::unordered_map<size_t, Base<T>*> gMapHashToType;

		template<class = IsDerived<Base<T>, T>>
		static std::unordered_map<size_t, std::string> gMapHashToClassName;
	};

	template<class T>
	class Type {
	public:
		static std::string_view name() {
			return class_identifier<T>();
		}

		static std::string_view type() {
			return class_type<T>();
		}

		static std::string_view full_name() {
			return class_name<T>();
		}

		static size_t hash_code() {
			return class_hash<T>();
		}
	};
}
}
