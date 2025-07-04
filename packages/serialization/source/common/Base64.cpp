#include "serialization/Base64.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <sstream>

namespace l::serialization {

	std::string base64_encode(unsigned char* src, size_t len) {
		std::string out;

		int val = 0, valb = -6;
		for (size_t i = 0; i < len; i++) {
			unsigned char c = src[i];
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
		}
		while (out.size() % 4) {
			out.push_back('=');
		}
		return out;
	}

	void base64_decode(unsigned char* dst, std::string_view src) {
		std::vector<int> T(256, -1);
		for (uint8_t i = 0; i < 64; i++) {
			T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
		}

		int i = 0;
		int val = 0, valb = -8;
		for (unsigned char c : src) {
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				dst[i++] = static_cast<unsigned char>((val >> valb) & 0xff);
				valb -= 8;
			}
		}
	}

	// Should follow these rules: https://www.rfc-editor.org/rfc/rfc3548
	std::string base64_encode(std::string_view in) {
		std::string out;

		int val = 0, valb = -6;
		for (unsigned char c : in) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
		}
		while (out.size() % 4) {
			out.push_back('=');
		}
		return out;
	}

	std::string base64_decode(std::string_view in) {
		std::string out;

		std::vector<int> T(256, -1);
		for (uint8_t i = 0; i < 64; i++) {
			T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
		}

		int val = 0, valb = -8;
		for (unsigned char c : in) {
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}


}
