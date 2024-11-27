#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "logging/String.h"

namespace l::serialization {
	std::string baseascii_encode(unsigned char* src, size_t len, bool tolowercase = true);
	void baseascii_decode(unsigned char* dst, std::string_view src);

	std::string baseascii_encode(std::string_view in, bool tolowercase = true);
	std::string baseascii_decode(std::string_view in);
}
