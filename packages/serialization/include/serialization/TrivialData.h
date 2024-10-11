#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "logging/String.h"

namespace l::serialization {

	template<size_t size>
	std::pair<size_t, std::vector<std::string>> ParseTrivialData(std::stringstream& data, std::string_view separators) {

		std::vector<std::string> out;

		size_t counter = 0;
		for (std::string line; std::getline(data, line, '\n');) {
			auto elements = l::string::split(line, separators);
			for (auto it : elements) {
				out.push_back(std::string(it));
			}
		}

		return {size, out};
	}

}
