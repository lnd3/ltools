#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "logging/String.h"

namespace l::serialization {
	std::string base64_encode(const std::string& in);
	std::string base64_decode(const std::string& in);
}
