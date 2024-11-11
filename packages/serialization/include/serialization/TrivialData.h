#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "logging/String.h"

namespace l::serialization {
	std::vector<std::string> ParseTrivialData(std::stringstream& data, std::string_view separators);
	std::unordered_map<uint32_t, std::string> ParseTrivialDataMap(std::stringstream& data, std::string_view separators);
}
