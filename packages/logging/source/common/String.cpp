#include "logging/String.h"
#include "logging/Log.h"

#include <algorithm>
#include <iterator>
#include <regex>
#include <locale>
#include <iostream>
#include <chrono>
#include <ctime>   // localtime
#include <codecvt>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <time.h>

namespace {
	constexpr size_t buffer_size = 1024;

	std::mutex bufferMutex;
	char buffer[buffer_size];

	std::mutex wbufferMutex;
	wchar_t wbuffer[buffer_size];

}

namespace l {
namespace string {

	int32_t get_local_timezone() {
#ifdef WIN32
		long time;
		auto res = _get_timezone(&time);
		ASSERT(res == 0);
#else
		auto time = __timezone;
#endif
		return static_cast<int32_t>(- time); // negate since timezone is how to get utc time from local time (local time - utc time)
	}

	int32_t get_local_daylight_savings(bool inHours) {
#ifdef WIN32
		int time;
		auto res = _get_daylight(&time);
		ASSERT(res == 0);
#else
		auto time = __daylight;
#endif
		return static_cast<int32_t>(inHours ? time : time * 3600);
	}

	time_t convert_to_local_time_from_utc_time(const time_t time) {
		auto daylight = get_local_daylight_savings();
		auto timezone = get_local_timezone();
		return time + timezone + daylight;
	}

	time_t convert_to_utc_time_from_local_time(const time_t time) {
		auto daylight = get_local_daylight_savings();
		auto timezone = get_local_timezone();
		return time - timezone - daylight;
	}

	time_t convert_to_time(const tm* timeinfo, bool adjustYearAndMonth) {
		tm timeinfo2 = *timeinfo;
		if (adjustYearAndMonth) {
			timeinfo2.tm_year -= 1900;
			timeinfo2.tm_mon -= 1;
		}
#ifdef WIN32
		// Converts a UTC time represented by a struct tm to a UTC time represented by a time_t type.
		// i.e does not correct for time zone
		time_t time = _mkgmtime64(&timeinfo2);
#else
		time_t time = timegm(&timeinfo2);
#endif
		return time;
	}

	void convert_to_tm(const time_t time, tm* timeinfo, bool adjustYearAndMonth) {
#ifdef WIN32
		auto res = _gmtime64_s(timeinfo, &time);
		ASSERT(res == 0);
#else
		tm* ti = gmtime(&time);
		*timeinfo = *ti;
#endif

		if (adjustYearAndMonth) {
			timeinfo->tm_year += 1900;
			timeinfo->tm_mon += 1;
		}
	}

	time_t convert_to_utc_time_from_local_tm(const tm* utctimeinfo, bool adjustYearAndMonth) {
		tm timeinfo = *utctimeinfo;
		if (adjustYearAndMonth) {
			timeinfo.tm_year -= 1900;
			timeinfo.tm_mon -= 1;
		}
#ifdef WIN32
		// Convert the local time to a calendar value.
		time_t time = _mktime64(&timeinfo);
#else
		time_t time = mktime(&timeinfo);
#endif
		return time;
	}

	void convert_to_local_tm_from_utc_time(const time_t utctime, tm* timeinfo, bool adjustYearAndMonth) {
#ifdef WIN32
		// Converts a time_t time value to a tm structure, and corrects for the local time zone. 
		auto res = _localtime64_s(timeinfo, &utctime);
#else
		auto res = localtime_r(&utctime, timeinfo);
#endif
		if (adjustYearAndMonth) {
			timeinfo->tm_year += 1900;
			timeinfo->tm_mon += 1;
		}
		ASSERT(res == 0);
	}

	int32_t get_unix_epoch() {
		return static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000LL);
	}

	int64_t get_unix_epoch_ms() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	int32_t to_unix_time(int year, int month, int day, int hour, int min, int sec) {
		struct tm timeinfo = {};

		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;    //months since January - [0,11]
		timeinfo.tm_mday = day;          //day of the month - [1,31] 
		timeinfo.tm_hour = hour;         //hours since midnight - [0,23]
		timeinfo.tm_min = min;          //minutes after the hour - [0,59]
		timeinfo.tm_sec = sec;          //seconds after the minute - [0,59]

		// use gmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		return static_cast<int32_t>(convert_to_time(&timeinfo));
	}

	int32_t to_unix_time(std::string_view date) {
		struct tm timeinfo = {};

		int ret = 0;

		if (date.size() > 10) {
			ASSERT(date.size() == 19);
#ifdef WIN32
			ret = sscanf_s(date.data(), "%4d-%2d-%2d %2d:%2d:%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#else
			ret = sscanf(date.data(), "%4d-%2d-%2d %2d:%2d:%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#endif
			ASSERT(ret <= 6);
		}
		else {
			ASSERT(date.size() == 10);
#ifdef WIN32
			ret = sscanf_s(date.data(), "%4d-%2d-%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday);
#else
			ret = sscanf(date.data(), "%4d-%2d-%2d",
				&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday);
#endif
			timeinfo.tm_hour = 0;
			timeinfo.tm_min = 0;
			timeinfo.tm_sec = 0;
			ASSERT(ret <= 3);
		}


		timeinfo.tm_year -= 1900;
		timeinfo.tm_mon -= 1;

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		return static_cast<int32_t>(convert_to_time(&timeinfo));
	}

	int32_t to_unix_time2(std::string_view date) {
		struct tm timeinfo = {};

		int ret = 0;
		int microsec;

		ASSERT(date.size() == 28);
#ifdef WIN32
		ret = sscanf_s(date.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#else
		ret = sscanf(date.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#endif

		ASSERT(ret <= 7);

		timeinfo.tm_year -= 1900;
		timeinfo.tm_mon -= 1;

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		return static_cast<int32_t>(convert_to_time(&timeinfo));
	}

	int32_t to_unix_time_from_local(const int32_t* dateAndTime) {
		struct tm timeinfo = {};
		timeinfo.tm_year = dateAndTime[0];
		timeinfo.tm_mon = dateAndTime[1];
		timeinfo.tm_mday = dateAndTime[2];
		timeinfo.tm_hour = dateAndTime[3];
		timeinfo.tm_min = dateAndTime[4];
		timeinfo.tm_sec = dateAndTime[5];
		timeinfo.tm_isdst = get_local_daylight_savings(true);

		bool adjustYearAndMonth = timeinfo.tm_year > 1000 ? true : false;
		return static_cast<int32_t>(convert_to_utc_time_from_local_tm(&timeinfo, adjustYearAndMonth));
	}

	int32_t to_unix_time_from_local(const tm& timeinfo) {
		bool adjustYearAndMonth = timeinfo.tm_year > 1000 ? true : false;
		return static_cast<int32_t>(convert_to_utc_time_from_local_tm(&timeinfo, adjustYearAndMonth));
	}

	int32_t to_unix_time_local(std::string_view dateAndTime) {
		struct tm timeinfo = {};

#ifdef WIN32
		int ret = sscanf_s(dateAndTime.data(), "%4d-%2d-%2d %2d:%2d:%2d",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#else
		int ret = sscanf(dateAndTime.data(), "%4d-%2d-%2d %2d:%2d:%2d",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
#endif

		ASSERT(ret <= 6);

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		bool adjustYearAndMonth = timeinfo.tm_year > 1000 ? true : false;
		return static_cast<int32_t>(convert_to_utc_time_from_local_tm(&timeinfo), adjustYearAndMonth);
	}

	int32_t to_unix_time_local2(std::string_view dateAndTime) {
		struct tm timeinfo = {};
		int microsec;
		ASSERT(dateAndTime.size() == 28);
#ifdef WIN32
		int ret = sscanf_s(dateAndTime.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#else
		int ret = sscanf(dateAndTime.data(), "%4d-%2d-%2dT%2d:%2d:%2d.%7dZ",
			&timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, &microsec);
#endif

		ASSERT(ret <= 7);

		// use _mkgmtime for gmt/utc time, use it when local time zone is unknown, for example in storage
		// use mktime for local time zone presentation
		bool adjustYearAndMonth = timeinfo.tm_year > 1000 ? true : false;
		return static_cast<int32_t>(convert_to_utc_time_from_local_tm(&timeinfo), adjustYearAndMonth);
	}

	void to_local_time(const int32_t unixtime, int32_t* dateAndTime) {
		tm timeinfo;
		convert_to_local_tm_from_utc_time(unixtime, &timeinfo);
		dateAndTime[0] = timeinfo.tm_year;
		dateAndTime[1] = timeinfo.tm_mon;
		dateAndTime[2] = timeinfo.tm_mday;
		dateAndTime[3] = timeinfo.tm_hour;
		dateAndTime[4] = timeinfo.tm_min;
		dateAndTime[5] = timeinfo.tm_sec;
	}

	std::string get_local_time_string(const int32_t unixtime, std::string_view format) {
		struct std::tm tminfo = {};

		convert_to_local_tm_from_utc_time(unixtime, &tminfo);

		std::ostringstream out;
		out << std::put_time(&tminfo, format.data());
		return out.str();
	}

	size_t get_local_time_string_verbose(char* buf, size_t maxSize) {
		using namespace std::chrono;

		auto n = system_clock::now();
		auto tp = n.time_since_epoch();

		auto micros = std::chrono::duration_cast<std::chrono::microseconds>(tp);
		auto seconds = std::chrono::duration_cast<std::chrono::hours>(tp);
		auto minutes = std::chrono::duration_cast<std::chrono::hours>(tp);
		auto hours = std::chrono::duration_cast<std::chrono::hours>(tp);

		const time_t now = system_clock::to_time_t(n);
		struct tm newtime = {};

		convert_to_local_tm_from_utc_time(now, &newtime);

		auto micro = static_cast<int>(micros.count() % 1000000);
		auto count = std::snprintf(buf, maxSize, "%.2d-%.2d-%.2d %.2d:%.2d:%.2d.%.6d", newtime.tm_year, newtime.tm_mon, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec, micro);

		return static_cast<size_t>(count);
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

	std::string narrow(const std::wstring& str) {
		// http://www.cplusplus.com/reference/locale/ctype/narrow/
		std::locale loc;

		auto size = str.length();
		EXPECT(size > 0 && size < buffer_size) << "Failed to narrow string of size " << size << " characters";

		auto str_ptr = str.data();

		std::lock_guard<std::mutex> lock(bufferMutex);
		std::use_facet< std::ctype<wchar_t> >(loc).narrow(str_ptr, str_ptr + size + 1, '?', buffer);
		return std::string(buffer, size);
	}

	std::wstring widen(const std::string& str) {
		// http://www.cplusplus.com/reference/locale/ctype/widen/

		std::locale loc("");

		auto size = str.length();
		EXPECT(size > 0 && size < buffer_size) << "Failed to widen string of size " << size << " characters";

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