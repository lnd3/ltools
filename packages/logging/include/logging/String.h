#pragma once

#include "logging/Log.h"

#include <locale>
#include <codecvt>
#include <string.h>
#include <vector>
#include <string_view>
#include <string>

namespace l {
namespace string {
	int32_t get_local_timezone();
	int32_t get_local_daylight_savings(bool inHours = false);

	time_t convert_to_local_time_from_utc_time(const time_t time);
	time_t convert_to_utc_time_from_local_time(const time_t time);

	void convert_to_tm(const time_t time, tm* timeinfo, bool adjustYearAndMonth = true);
	time_t convert_to_time(const tm* timeinfo, bool adjustYearAndMonth = true);
	void convert_to_local_tm_from_utc_time(const time_t utctime, tm* localtimeinfo, bool adjustYearAndMonth = true);
	time_t convert_to_utc_time_from_local_tm(const tm* localtimeinfo, bool adjustYearAndMonth = true);

	int32_t get_unix_epoch();
	int64_t get_unix_epoch_ms();

	int32_t to_unix_time(int year, int month, int day, int hour = 0, int min = 0, int sec = 0);
	int32_t to_unix_time(std::string_view date = "2024-01-18 14:04:00");
	int32_t to_unix_time2(std::string_view date = "2024-01-18T14:04:00000Z");

	int32_t to_unix_time_from_local(const int32_t* dateAndTime);
	int32_t to_unix_time_from_local(const tm& timeinfo);
	int32_t to_unix_time_local(std::string_view utcdate = "2024-01-18 14:04:00");
	int32_t to_unix_time_local2(std::string_view utcdate = "2024-01-18T14:04:00000Z");

	void to_local_time(const int32_t unixtime, int32_t* fullDateAndTime);
	std::string get_local_time_string(const int32_t unixtime, std::string_view format = "%Y-%m-%d %X");

	size_t get_local_time_string_verbose(char* buf, size_t maxSize);
	
	uint32_t string_id(std::string_view string);

	std::string encode_html(const std::string& input);

	template<class T>
	T get_number(std::string_view number) {
		char buffer[24];
		ASSERT(number.size() < 24);
		memcpy(buffer, number.data(), number.size());
		buffer[number.size()] = '\0';
		if constexpr (std::is_floating_point_v<T>) {
			return static_cast<T>(std::atof(buffer));
		}
		else if constexpr (std::is_integral_v<T>) {
			return static_cast<T>(std::atoi(buffer));
		}
		else {
			ASSERT(false);
			return 0;
		}
	}

	bool equal(const char* a, const char* b, size_t a_offset = 0, size_t b_offset = 0, size_t maxCount = 20);
	bool equal_partial(const char* a, const char* b, size_t a_offset = 0, size_t b_offset = 0, size_t maxCount = 20);
	bool equal_partial(std::string_view a, std::string_view b, size_t a_offset = 0, size_t b_offset = 0);

	std::vector<std::wstring_view> split(std::wstring_view text, std::wstring_view delim = L" \t\n", char escapeChar = '\"');
	std::vector<std::string_view> split(std::string_view text, std::string_view delim = " \t\n", char escapeChar = '\"');

	std::string narrow(const std::wstring& str);
	std::wstring widen(const std::string& str);

	int count_digits(int number);

	template<class T>
	concept Number = requires(T a) { std::convertible_to<T, float> || std::convertible_to<T, uint32_t>; };
	template<Number T>
	void cstring_to_numbers(const char* src, size_t count, char separator, std::vector<T>& dst) {
		constexpr size_t bufferSize = 1024;
		std::array<T, bufferSize> buffer;
		std::array<char, 32> charBuffer{0};
		auto srcPtr = const_cast<char*>(src);

		for (size_t i = 0; i < count; i += bufferSize) {
			auto currentBatchSize = count - i > bufferSize ? bufferSize : count - i;
			for (size_t j = 0; j < currentBatchSize; j++) {
				size_t k = 0;
				while (*srcPtr != separator) {
					charBuffer[k++] = *srcPtr++;
				}
				srcPtr++;
				charBuffer[k++] = '\0';

				//ASSERT(k < 32) << "Failed to fit number in char buffer";

				if constexpr (std::is_integral<T>::value) {
					buffer[j] = static_cast<uint32_t>(std::atoi(charBuffer.data()));
				}
				else {
					buffer[j] = static_cast<float>(std::atof(charBuffer.data()));
				}
			}
			dst.insert(dst.end(), buffer.begin(), buffer.begin() + currentBatchSize);
		}
	}

	template<const char pad = '0'>
	std::string to_fixed_string(int number, int fixed) {
		std::string prefix = "";
		int count = count_digits(number);
		constexpr const char pad1[] = { pad , 0 };
		constexpr const char pad2[] = { pad, pad , 0 };
		constexpr const char pad3[] = { pad, pad, pad , 0 };
		constexpr const char pad4[] = { pad, pad, pad, pad , 0 };
		constexpr const char pad5[] = { pad, pad, pad, pad, pad , 0 };
		constexpr const char pad6[] = { pad, pad, pad, pad, pad, pad , 0 };
		constexpr const char pad7[] = { pad, pad, pad, pad, pad, pad, pad , 0 };
		constexpr const char pad8[] = { pad, pad, pad, pad, pad, pad, pad, pad , 0 };
		constexpr const char pad9[] = { pad, pad, pad, pad, pad, pad, pad, pad, pad , 0 };
		constexpr const char pad10[] = { pad, pad, pad, pad, pad, pad, pad, pad, pad, pad , 0 };
		constexpr const char pad11[] = { pad, pad, pad, pad, pad, pad, pad, pad, pad, pad, pad , 0 };
		constexpr const char pad12[] = { pad, pad, pad, pad, pad, pad, pad, pad, pad, pad, pad, pad , 0 };

		switch (fixed - count) {
		case 0: return std::to_string(number);
		case 1: return pad1 + std::to_string(number);
		case 2: return pad2 + std::to_string(number);
		case 3: return pad3 + std::to_string(number);
		case 4: return pad4 + std::to_string(number);
		case 5: return pad5 + std::to_string(number);
		case 6: return pad6 + std::to_string(number);
		case 7: return pad7 + std::to_string(number);
		case 8: return pad8 + std::to_string(number);
		case 9: return pad9 + std::to_string(number);
		case 10: return pad10 + std::to_string(number);
		case 11: return pad11 + std::to_string(number);
		case 12: return pad12 + std::to_string(number);
		default: return std::to_string(number);
		}
	}

	template <class I, class = std::enable_if_t<std::is_integral_v<I>>>
	std::string to_hex(I w, size_t hex_len = sizeof(I) << 1) {
		static const char* digits = "0123456789ABCDEF";
		std::string rc(hex_len, '0');
		for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4) {
			rc[i] = digits[(w >> j) & 0x0f];
		}
		return rc;
	}

	std::string_view cut(std::string_view s, const char ch);
	std::wstring_view cut(std::wstring_view s, const wchar_t ch);

	std::string_view rcut(std::string_view s, const char ch);
	std::wstring_view rcut(std::wstring_view s, const wchar_t ch);
}
}