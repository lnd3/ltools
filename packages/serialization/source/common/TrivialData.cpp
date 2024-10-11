#include "logging/String.h"

#include "serialization/TrivialData.h"

#include <string>
#include <vector>
#include <sstream>

namespace l::serialization {

	std::vector<std::string> ParseTrivialData(std::stringstream& data, std::string_view separators) {

		std::vector<std::string> out;

		for (std::string line; std::getline(data, line, '\n');) {
			auto elements = l::string::split(line, separators);
			for (auto it : elements) {
				out.push_back(std::string(it));
			}
		}

		return out;
	}

	std::unordered_map<uint32_t, std::string> ParseTrivialDataMap(std::stringstream& data, std::string_view separators) {

		auto out = ParseTrivialData(data, separators);

		std::unordered_map<uint32_t, std::string> map;

		for (int i = 0; i < out.size() / 2; i++) {
			uint32_t key = l::string::string_id(out[i * 2]);
			map[key] = out[i * 2 + 1];
		}

		return map;
	}

}
