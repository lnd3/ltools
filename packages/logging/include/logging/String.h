#pragma once

#include "logging/Log.h"

#include <locale>
#include <codecvt>
#include <string.h>
#include <vector>
#include <string_view>
#include <string>
#include <memory>

namespace l::string {

	template<class T>
	void convert(std::stringstream& dst, std::vector<T>& src, size_t count = 0) {
		static_assert(sizeof(T) == sizeof(char));
		dst.write(reinterpret_cast<char*>(src.data()), count > 0 ? count : src.size());
	}

	template<class T, const size_t SIZE>
	void convert(std::stringstream& dst, std::array<T, SIZE>& src, size_t count = 0) {
		static_assert(sizeof(T) == sizeof(char));
		dst.write(reinterpret_cast<char*>(src.data()), count > 0 ? count : src.size());
	}

	template<class T>
	void convert(std::vector<T>& dst, std::stringstream& src) {
		T tmp{};
		static_assert(sizeof(T) == sizeof(char));
		while (src >> tmp) dst.push_back(tmp);
	}

	template<int32_t BUFSIZE>
	class string_buffer {
	public:
		string_buffer() = default;
		~string_buffer() = default;

		void pos(int32_t p) {
			mPos = (p < 0) ? 0 : (p >= BUFSIZE ? BUFSIZE - 1 : p);
		}

		void clear() {
			mPos = 0;
		}

		size_t left() {
			return static_cast<size_t>(BUFSIZE - mPos);
		}

		char& cur() {
			return mBuf[size()];
		}
		size_t size() {
			return static_cast<size_t>(mPos);
		}

		int32_t append(std::string_view s) {
			int32_t count = static_cast<int32_t>(s.size() < left() ? s.size() : left() - 1);
			memcpy(&cur(), s.data(), static_cast<size_t>(count));
			mPos += count;
			cur() = 0;
			return count;
		}

		template<class ...T>
		int32_t printf(const char* format, T ...args) {
			int count = std::snprintf(&cur(), left(), format, std::forward<T>(args)...);
			mPos += count;
			return count;
		}

		std::string_view str() {
			return std::string_view( &mBuf[0], size());
		}

	protected:
		int32_t mPos = 0;
		char mBuf[BUFSIZE];
	};

	class stackstringview {
	public:
		stackstringview() = default;
		~stackstringview() = default;

		std::string_view view() {
			return std::string_view(mBufPtr, mSize);
		}
	protected:
		size_t mSize = 0;
		char* mBufPtr = nullptr;
	};

	template<size_t SIZE>
	class stackstring : stackstringview {
	public:
		stackstring() {
			mSize = SIZE - 1u;
			mBufPtr = &mBuf[0];
			memset(mBufPtr, 0, SIZE);
		}
		~stackstring() = default;

		stackstring(const stackstring& other) : stackstring() {
			auto size = other.mSize <= mSize ? other.mSize : mSize;
			memcpy(mBufPtr, &other.mBufPtr, size);
			mBuf[size] = 0;
		}
		stackstring& operator=(const stackstring& other) {
			auto size = other.mSize <= mSize ? other.mSize : mSize;
			memcpy(mBufPtr, other.mBufPtr, size);
			mBuf[size] = 0;
			return *this;
		}
	protected:
		char mBuf[SIZE];
	};

	void init_timezone();
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
	
	template<size_t BUFSIZE>
	void get_local_date(string_buffer<BUFSIZE>& buf, const int32_t unixtime, bool fullYear = false) {
		struct std::tm tminfo = {};
		convert_to_local_tm_from_utc_time(unixtime, &tminfo, false);
		if (fullYear) {
			buf.printf("%4d-%2d-%2d", tminfo.tm_year, tminfo.tm_mon + 1, tminfo.tm_mday);
		}
		else {
			buf.printf("%4d-%2d-%2d", tminfo.tm_year, tminfo.tm_mon + 1, tminfo.tm_mday);
		}
	}

	template<size_t BUFSIZE>
	void get_local_time(string_buffer<BUFSIZE>& buf, const int32_t unixtime) {
		struct std::tm tminfo = {};
		convert_to_local_tm_from_utc_time(unixtime, &tminfo, false);
		buf.printf("%2d:%2d:%2d", tminfo.tm_hour, tminfo.tm_min, tminfo.tm_sec);
	}

	template<size_t BUFSIZE>
	void get_local_date_and_time(string_buffer<BUFSIZE>& buf, const int32_t unixtime, bool fullYear = false) {
		struct std::tm tminfo = {};
		convert_to_local_tm_from_utc_time(unixtime, &tminfo, false);
		if (fullYear) {
			buf.printf("%4d-%2d-%2d %2d:%2d:%2d", tminfo.tm_year, tminfo.tm_mon + 1, tminfo.tm_mday, tminfo.tm_hour, tminfo.tm_min, tminfo.tm_sec);
		}
		else {
			buf.printf("%2d-%2d-%2d %2d:%2d:%2d", tminfo.tm_year, tminfo.tm_mon + 1, tminfo.tm_mday, tminfo.tm_hour, tminfo.tm_min, tminfo.tm_sec);
		}
	}

	template<size_t SIZE>
	uint32_t string_id(const char(&string)[SIZE]) {
		std::hash<std::string_view> hasher;
		auto id = hasher(std::string_view(&string[0], SIZE-1));
		return static_cast<uint32_t>(id);
	}
	uint32_t string_id(std::string_view string);
	uint32_t string_id(const std::string& string);

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
	bool equal_anywhere(std::string_view a, std::string_view b);

	std::vector<std::wstring_view> split(std::wstring_view text, std::wstring_view delim = L" \t\n", char escapeChar = '\"');
	std::vector<std::string_view> split(std::string_view text, std::string_view delim = " \t\n", char escapeChar = '\"');

	std::string narrow(const std::wstring& str);
	std::wstring widen(const std::string& str);

	int count_digits(int number);

	template<class T>
	concept Number = requires(T a) { requires std::convertible_to<T, float> || std::convertible_to<T, uint32_t>; };
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

	std::string to_hex2(unsigned char* src, size_t hex_len);
	std::string to_hex2(std::string_view str);

	std::string hex_encode(unsigned char* src, size_t len);
	void hex_decode(unsigned char* dst, std::string_view src);

	std::string hex_encode(std::string_view str);
	std::string hex_decode(std::string_view str);
}

