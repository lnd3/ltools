#include "logging/String.h"
#include "logging/Log.h"

#include <algorithm>
#include <iterator>
#include <regex>
#include <locale>
#include <iostream>
#include <chrono>
#include <ctime>   // localtime

namespace {
	constexpr size_t buffer_size = 1024;

	std::mutex bufferMutex;
	char buffer[buffer_size];

	std::mutex wbufferMutex;
	wchar_t wbuffer[buffer_size];
}

namespace l {
namespace string {

	tm get_time_info(int32_t unixtime, bool adjustYearAndMonth) {
		struct tm timeinfo;
		const time_t time = static_cast<time_t>(unixtime);
#ifdef WIN32
		auto res = localtime_s(&timeinfo, &time);
#else
		auto res = localtime_r(&time, &timeinfo);
#endif
		ASSERT(res == 0);
		if (adjustYearAndMonth) {
			timeinfo.tm_year += 1900;
			timeinfo.tm_mon += 1;
		}
		return timeinfo;
	}

	int32_t get_unix_time(const tm& timeinfo, bool adjustYearAndMonth) {
		struct tm ti = timeinfo;
		if (adjustYearAndMonth) {
			ti.tm_year -= 1900;
			ti.tm_mon -= 1;
		}
		auto date = mktime(&ti);
		return static_cast<int32_t>(date);
	}

	void get_time_info(int32_t* fullDateAndTime, int32_t unixtime) {
		auto timeinfo = l::string::get_time_info(unixtime, true);
		fullDateAndTime[0] = timeinfo.tm_year;
		fullDateAndTime[1] = timeinfo.tm_mon;
		fullDateAndTime[2] = timeinfo.tm_mday;
		fullDateAndTime[3] = timeinfo.tm_hour;
		fullDateAndTime[4] = timeinfo.tm_min;
		fullDateAndTime[5] = timeinfo.tm_sec;
	}

	int32_t get_unix_time(int32_t* fullDateAndTime) {
		struct tm timeinfo;
		timeinfo.tm_year = fullDateAndTime[0];
		timeinfo.tm_mon = fullDateAndTime[1];
		timeinfo.tm_mday = fullDateAndTime[2];
		timeinfo.tm_hour = fullDateAndTime[3];
		timeinfo.tm_min = fullDateAndTime[4];
		timeinfo.tm_sec = fullDateAndTime[5];
		return get_unix_time(timeinfo, true);
	}

	size_t get_time_string(char* buf, size_t maxSize) {
		using namespace std::chrono;

		auto n = system_clock::now();
		auto tp = n.time_since_epoch();

		auto micros = std::chrono::duration_cast<std::chrono::microseconds>(tp);
		auto seconds = std::chrono::duration_cast<std::chrono::hours>(tp);
		auto minutes = std::chrono::duration_cast<std::chrono::hours>(tp);
		auto hours = std::chrono::duration_cast<std::chrono::hours>(tp);

		const time_t now = system_clock::to_time_t(n);
		struct tm newtime;
#ifdef WIN32
		localtime_s(&newtime, &now);
#else
		localtime_r(&now, &newtime);
#endif



		newtime.tm_year += 1900;
		auto micro = static_cast<int>(micros.count() % 1000000);
		auto count = std::snprintf(buf, maxSize, "%.2d-%.2d-%.2d %.2d:%.2d:%.2d.%.6d", newtime.tm_year, newtime.tm_mon, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec, micro);

		return static_cast<size_t>(count);
	}

	std::string get_time_string(const int64_t unixtime, std::string_view format) {
		struct std::tm tminfo {};
#ifdef WIN32
		localtime_s(&tminfo, &unixtime);
#else
		localtime_r(&unixtime, &tminfo);
#endif

		std::ostringstream out;
		out << std::put_time(&tminfo, format.data());
		return out.str();
	}

	int32_t get_unix_timestamp() {
		return static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000LL);
	}

	int64_t get_unix_timestamp_ms() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	int32_t to_unix_time(std::string_view utc_date) {
		struct tm timeinfo;

		int ret = 0;

		if (utc_date.size() > 10) {
			ASSERT(utc_date.size() == 19);
#ifdef WIN32
			ret = sscanf_s(utc_date.data(), "%4d-%2d-%2d %2d:%2d:%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#else
			ret = sscanf(utc_date.data(), "%4d-%2d-%2d %2d:%2d:%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#endif
		}
		else {
			ASSERT(utc_date.size() == 10);
#ifdef WIN32
			ret = sscanf_s(utc_date.data(), "%4d-%2d-%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday);
#else
			ret = sscanf(utc_date.data(), "%4d-%2d-%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday);
#endif
			timeinfo.tm_hour = 0;
			timeinfo.tm_min = 0;
			timeinfo.tm_sec = 0;
		}

		ASSERT(ret == 6);

		timeinfo.tm_year -= 1900;
		timeinfo.tm_mon -= 1;

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
#ifdef WIN32
		auto unix_time = _mkgmtime(&timeinfo);
#else
		auto unix_time = mktime(&timeinfo);
#endif

		return static_cast<int32_t>(unix_time);
	}

	int32_t to_unix_time2(std::string_view utc_date) {
		struct tm timeinfo;

		int ret = 0;
		int microsec;

		ASSERT(utc_date.size() == 28);
#ifdef WIN32
		ret = sscanf_s(utc_date.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#else
		ret = sscanf(utc_date.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#endif

		ASSERT(ret == 6);

		timeinfo.tm_year -= 1900;
		timeinfo.tm_mon -= 1;

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
#ifdef WIN32
		auto unix_time = _mkgmtime(&timeinfo);
#else
		auto unix_time = mktime(&timeinfo);
#endif

		return static_cast<int32_t>(unix_time);
	}

	int32_t to_local_unix_time(std::string_view utc_date) {
		struct tm timeinfo;

#ifdef WIN32
		int ret = sscanf_s(utc_date.data(), "%4d-%2d-%2d %2d:%2d:%2d",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#else
		int ret = sscanf(utc_date.data(), "%4d-%2d-%2d %2d:%2d:%2d",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#endif

		ASSERT(ret == 6);

		timeinfo.tm_year -= 1900;
		timeinfo.tm_mon -= 1;

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		auto unix_time = mktime(&timeinfo);

		return static_cast<int32_t>(unix_time);
	}

	int32_t to_local_unix_time2(std::string_view utc_date) {
		struct tm timeinfo;
		int microsec;
		ASSERT(utc_date.size() == 28);
#ifdef WIN32
		int ret = sscanf_s(utc_date.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#else
		int ret = sscanf(utc_date.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#endif

		ASSERT(ret == 6);

		timeinfo.tm_year -= 1900;
		timeinfo.tm_mon -= 1;

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		auto unix_time = mktime(&timeinfo);

		return static_cast<int32_t>(unix_time);
	}

	int32_t to_unix_time(int year, int month, int day, int hour, int min, int sec) {
		struct tm timeinfo;

		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;    //months since January - [0,11]
		timeinfo.tm_mday = day;          //day of the month - [1,31] 
		timeinfo.tm_hour = hour;         //hours since midnight - [0,23]
		timeinfo.tm_min = min;          //minutes after the hour - [0,59]
		timeinfo.tm_sec = sec;          //seconds after the minute - [0,59]

		// use gmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
#ifdef WIN32
		auto date = _mkgmtime(&timeinfo);
#else
		auto date = mktime(&timeinfo);
#endif

		return static_cast<int32_t>(date);
	}

	bool cstring_equal(const char* a, const char* b, size_t a_offset, size_t b_offset) {
		return !strcmp(a + a_offset, b + b_offset);
	}

	bool partial_equality(const char* a, const char* b, size_t a_offset, size_t b_offset) {
		for (size_t i = 0; i < a_offset; i++) { // check for missed null terminators before a_offset
			if (a[i] == 0) {
				return false;
			}
		}
		for (size_t i = 0; i < b_offset; i++) { // check for missed null terminators before b_offset
			if (b[i] == 0) {
				return false;
			}
		}
		const char* a1 = a + a_offset;
		const char* b1 = b + b_offset;
		int i = 0;
		for (i = 0; a1[i] != 0 && b1[i] != 0; i++) {
			if (a1[i] != b1[i]) {
				return false;
			}
		}
		if (a1[i] != 0 && b1[i] != 0) {
			return false;
		}
		// One of the strings must have been searched fully for a partial match
		return true;
	}

	bool partial_equality(std::string_view a, std::string_view b, size_t a_offset, size_t b_offset) {
		size_t i = 0;
		for (i = 0; (i < a.size() - a_offset) && (i < b.size() - b_offset); i++) {
			if (a.data()[i + a_offset] != b.data()[i + b_offset]) {
				return false;
			}
		}
		if (a.size() < i + a_offset && b.size() < i + b_offset) {
			return false;
		}
		return true;
	}

	std::vector<std::wstring_view> split(std::wstring_view text, std::wstring_view delim, char escapeChar) {
		std::vector<std::wstring_view> out;

		auto insert = [&out](const wchar_t* data, size_t count) {
			if (count > 0) {
				out.emplace_back(data, count);
			}
		};
		bool escape = false;
		auto start = text.begin();
		for (auto it = text.begin(); it != text.end(); it++) {
			if (*it == escapeChar) {
				insert(&*start, it - start);
				start = it + 1;
				escape = !escape;
			}
			else if (!escape) {
				for (auto it2 = delim.begin(); it2 != delim.end(); it2++) {
					if (*it == *it2) {
						insert(&*start, it - start);
						start = it + 1;
						break;
					}
				}
			}
		}
		if (start < text.end()) {
			insert(&*start, text.end() - start);
		}
		return out;

	}

	std::vector<std::string_view> split(std::string_view text, std::string_view delim, char escapeChar) {
		std::vector<std::string_view> out;

		auto insert = [&out](const char* data, size_t count) {
			if (count > 0) {
				out.emplace_back(data, count);
			}
		};

		bool escape = false;
		auto start = text.begin();
		for (auto it = text.begin(); it != text.end(); it++) {
			if (*it == escapeChar) {
				insert(&*start, it - start);
				start = it + 1;
				escape = !escape;
			}
			else if(!escape){
				for (auto it2 = delim.begin(); it2 != delim.end(); it2++) {
					if (*it == *it2) {
						insert(&*start, it - start);
						start = it + 1;
						break;
					}
				}
			}
		}
		if (start < text.end()) {
			insert(&*start, text.end() - start);
		}

		return out;
	}

	std::string narrow(std::wstring_view str) {
		// http://www.cplusplus.com/reference/locale/ctype/narrow/
		std::locale loc;

		auto size = str.size();
		EXPECT(size < buffer_size) << "Failed to narrow string longer than " << buffer_size << " characters";

		auto str_ptr = str.data();

		std::lock_guard<std::mutex> lock(bufferMutex);
		std::use_facet< std::ctype<wchar_t> >(loc).narrow(str_ptr, str_ptr + size + 1, '?', buffer);
		return std::string(buffer, size);
	}

	std::wstring widen(std::string_view str) {
		// http://www.cplusplus.com/reference/locale/ctype/widen/
		std::locale loc;

		auto size = str.size();
		EXPECT(size < buffer_size) << "Failed to widen string longer than " << buffer_size << " characters";

		auto str_ptr = str.data();

		std::lock_guard<std::mutex> lock(wbufferMutex);
		std::use_facet< std::ctype<wchar_t> >(loc).widen(str_ptr, str_ptr + size + 1, wbuffer);
		return std::wstring(wbuffer, size);
	}

	int count_digits(int number) {
		int i = 0;
		do {
			number /= 10;
			i++;
		} while (number != 0);
		return i;
	}

	std::string_view cut(std::string_view s, const char ch) {
		auto i = s.find(ch);
		if (i != std::string::npos) {
			return std::string_view(s.data(), i);
		}
		return s;
	}

	std::wstring_view cut(std::wstring_view s, const wchar_t ch) {
		auto i = s.find(ch);
		if (i != std::wstring::npos) {
			return std::wstring_view(s.data(), i);
		}
		return s;
	}

	std::string_view rcut(std::string_view s, const char ch) {
		auto i = s.rfind(ch) + 1;
		if (i != std::string::npos) {
			return std::string_view(s.data() + i, s.size() - i);
		}
		return s;
	}

	std::wstring_view rcut(std::wstring_view s, const wchar_t ch) {
		auto i = s.rfind(ch) + 1;
		if (i != std::wstring::npos) {
			return std::wstring_view(s.data() + i, s.size() - i);
		}
		return s;
	}

}
}