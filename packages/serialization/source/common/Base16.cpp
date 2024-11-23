#include "serialization/Base16.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <sstream>

namespace {
	const char s_vecUpper[] = "0123456789ABCDEF";
	const char s_vecLower[] = "0123456789abcdef";
	const char s_array[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};
}
namespace l::serialization {

	std::string base16_encode(unsigned char* src, size_t len, bool tolowercase) {
		std::string out;
		const char* svec = tolowercase ? s_vecLower : s_vecUpper;
		for (int i = 0; i < len; i++) {
			auto c = src[i];
			out += svec[(c >> 4) & 0xf];
			out += svec[(c & 0xf)];
		}
		return out;
	}

	void base16_decode(unsigned char* dst, std::string_view src) {
		auto size = src.size();
		size = size - size % 2;
		for (int i = 0; i < size; i+=2) {
			auto high = s_array[src.at(i + 0)];
			auto low = s_array[src.at(i + 1)];
			if (high >= 0 && low >= 0) {
				dst[i] = static_cast<unsigned char>((high << 4) | low);
			}
		}
	}

	std::string base16_encode(std::string_view src, bool tolowercase) {
		std::string out;
		const char* svec = tolowercase ? s_vecLower : s_vecUpper;
		for (int i = 0; i < src.size(); i++) {
			auto c = src[i];
			out += svec[(c >> 4) & 0xf];
			out += svec[(c & 0xf)];
		}
		return out;
	}

	std::string base16_decode(std::string_view src) {
		std::string out;
		auto size = src.size();
		size = size - size % 2;
		for (int i = 0; i < size; i += 2) {
			auto high = s_array[src.at(i + 0)];
			auto low = s_array[src.at(i + 1)];
			if (high >= 0 && low >= 0) {
				out += static_cast<unsigned char>((high << 4) | low);
			}
		}
		return out;
	}


}
