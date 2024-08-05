#include "meta/Reflection.h"

namespace {
	std::unordered_map<size_t, std::string> hash_to_name_mapping;
}

namespace l {
namespace meta {

	std::string_view class_name_from_hash(size_t hash_code) {
		auto it = hash_to_name_mapping.find(hash_code);
		if (it != hash_to_name_mapping.end()) {
			return it->second;
		}
		return "undefined";
	}

}
}
