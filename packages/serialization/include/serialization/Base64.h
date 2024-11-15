#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "logging/String.h"

namespace l::serialization {
	std::string base64_encode(std::string_view in);
	std::string base64_decode(std::string_view in);
}
