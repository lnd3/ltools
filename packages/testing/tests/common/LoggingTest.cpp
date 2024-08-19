#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

using namespace l;


TEST(Logging, StringSplit) {

	auto str = "aaa bbb			ccc\n\nddd";

	auto parts = string::split(str);

	TEST_TRUE(parts.at(0) == "aaa", "");
	TEST_TRUE(parts.at(1) == "bbb", "");
	TEST_TRUE(parts.at(2) == "ccc", "");
	TEST_TRUE(parts.at(3) == "ddd", "");

	return 0;
}

TEST(Logging, WideStringSplit) {

	auto str = L"aaa bbb			ccc\n\nddd";

	auto parts = string::split(str);

	TEST_TRUE(parts.at(0) == L"aaa", "");
	TEST_TRUE(parts.at(1) == L"bbb", "");
	TEST_TRUE(parts.at(2) == L"ccc", "");
	TEST_TRUE(parts.at(3) == L"ddd", "");

	return 0;
}

TEST(Logging, StringViewCutting) {
	auto str = "asdd aaaaaa:bbbb";

	auto cut = l::string::cut(str, ':');
	auto rcut = l::string::rcut(cut, ' ');

	TEST_TRUE(rcut == "aaaaaa", "Failed to extract string properly");

	return 0;
}

TEST(Logging, StringToHex) {
	auto out = string::to_hex(2);
	TEST_TRUE(out == "00000002", "");
	return 0;
}

#ifdef WIN32
TEST(Logging, StringCharacterConversion) {
	//auto s0 = L"ž © Õ ñ Ď Ĺ Ť Ɓ Ǯ Ƕ ɘ ʓ";
	auto s1 = std::wstring(L"© Õ ñ");
	LOG(LogInfo) << s1;
	auto s2 = string::narrow(s1);
	LOG(LogInfo) << s2;
	auto s3 = string::widen(s2);
	LOG(LogInfo) << s3;
	auto s4 = string::narrow(s3);
	LOG(LogInfo) << s4;

	TEST_TRUE(s3 == s1, "s1 did not equal s3");
	TEST_TRUE(s4 == s2, "s2 did not equal s4");

	return 0;
}
#endif

TEST(Logging, StringComparisons) {
	{
		std::string_view a = "abcdef";
		std::string_view b = "abc";
		std::string_view c = "cde";
		std::string_view d = "abcdf";
		std::string_view e = "cdf";
		std::string_view f = "fbcd";

		TEST_TRUE(l::string::partial_equality(a, b), "");
		TEST_TRUE(l::string::partial_equality(a, c, 2), "");
		TEST_FALSE(l::string::partial_equality(a, d), "");
		TEST_FALSE(l::string::partial_equality(a, e), "");

		TEST_TRUE(l::string::partial_equality(a, b, 1, 1), "");
		TEST_TRUE(l::string::partial_equality(a, c, 2, 0), "");
		TEST_FALSE(l::string::partial_equality(a, d, 3, 2), "");
		TEST_TRUE(l::string::partial_equality(d, e, 2, 0), "");
		TEST_TRUE(l::string::partial_equality(d, f, 1, 1), "");
	}

	{
		const char* a = "abcdef";
		const char* b = "abc";
		const char* c = "cde";
		const char* d = "abcdf";
		const char* e = "cdf";
		const char* f = "fbcd";

		TEST_TRUE(l::string::partial_equality(a, b), "");
		TEST_TRUE(l::string::partial_equality(a, c, 2), "");
		TEST_FALSE(l::string::partial_equality(a, d), "");
		TEST_FALSE(l::string::partial_equality(a, e), "");

		TEST_TRUE(l::string::partial_equality(a, b, 1, 1), "");
		TEST_TRUE(l::string::partial_equality(a, c, 2, 0), "");
		TEST_FALSE(l::string::partial_equality(a, d, 3, 2), "");
		TEST_TRUE(l::string::partial_equality(d, e, 2, 0), "");
		TEST_TRUE(l::string::partial_equality(d, f, 1, 1), "");
	}

	return 0;
}

TEST(Logging, TimeConversions) {
	{ // general time_t/tm conversion
		for (int i = 0; i < 2; i++) {
			bool adjustDate = i == 0 ? false : true;
			auto unixtime = l::string::get_unix_epoch();
			struct tm timeinfo;
			l::string::convert_to_tm(unixtime, &timeinfo, adjustDate);
			auto unixtime2 = l::string::convert_to_time(&timeinfo, adjustDate);
			TEST_EQ(unixtime, unixtime2, "");
		}
	}
	{ // utc to local time conversions
		for (int i = 0; i < 2; i++) {
			bool adjustDate = i == 0 ? false : true;
			auto unixtime = l::string::get_unix_epoch();
			struct tm timeinfo;
			l::string::convert_to_local_tm_from_utc_time(unixtime, &timeinfo, adjustDate);
			auto unixtime2 = l::string::convert_to_utc_time_from_local_tm(&timeinfo, adjustDate);
			TEST_EQ(unixtime, unixtime2, "");
		}
	}
	{
		auto unixtime = l::string::get_unix_epoch();
		auto localtime = l::string::convert_to_local_time_from_utc_time(unixtime);
		auto time = l::string::convert_to_utc_time_from_local_time(localtime);
		TEST_EQ(time, unixtime, "");
	}
	{
		auto unixtime = l::string::get_unix_epoch();

		auto localtime = l::string::convert_to_local_time_from_utc_time(unixtime);
		struct tm timeinfolocal;
		l::string::convert_to_tm(localtime, &timeinfolocal, true);

		struct tm timeinfo;
		l::string::convert_to_local_tm_from_utc_time(unixtime, &timeinfo, true);

		TEST_EQ(timeinfo.tm_hour, timeinfolocal.tm_hour, "");
	}

	{
		auto unixtime = l::string::get_unix_epoch();

		int32_t fullDate[6];
		l::string::to_local_time(unixtime, fullDate);

		auto unixtime2 = l::string::to_unix_time_from_local(fullDate);

		TEST_EQ(unixtime, unixtime2, "");
	}
	{
		auto unixtime = l::string::get_unix_epoch();
		int32_t fullDate[6];
		l::string::to_local_time(unixtime, fullDate);
		auto unixtimelocal = l::string::to_unix_time_from_local(fullDate);

	}

	return 0;
}

PERF_TEST(LoggingTest, LogTimings) {
	{
		PERF_TIMER("LogTimings::LogInfo");
		for (int i = 0; i < 10; i++) {
			LOG(LogInfo) << "Test logging";
		}
	}
	{
		PERF_TIMER("LogTimings::LogDebug");
		for (int i = 0; i < 10; i++) {
			LOG(LogDebug) << "Test logging";
		}
	}

	PERF_TIMER_RESULT("LogTimings");
	return 0;
}
