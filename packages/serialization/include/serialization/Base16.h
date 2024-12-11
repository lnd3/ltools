#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "logging/String.h"

namespace l::serialization {
	std::string base16_encode(unsigned char* src, size_t len, bool tolowercase = true);
	void base16_decode(unsigned char* dst, std::string_view src);

	std::string base16_encode(std::string_view in, bool tolowercase = true);
	std::string base16_decode(std::string_view in);
}
