#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <ctime>   // localtime
#include <string>
#include <vector>
#include <type_traits>
#include <utility>


#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK raise(SIGTRAP)
#endif

#ifndef _DEBUG
#undef DEBUG_BREAK
#define DEBUG_BREAK \
	(void)0
#endif

namespace l::string {
	std::string narrow(const std::wstring&);
}

template<class T>
std::underlying_type_t<T> implicit_cast(T&& value) {
	return static_cast<std::underlying_type_t<T>>(std::forward<T&&>(value));
}

namespace l {
namespace logging {
	size_t get_time_string(char* buffer, size_t maxSize);

	enum LogLevel : int32_t {
		LogDebug = 0,
		LogInfo,
		LogWarning,
		LogError,
		LogTest,
		LogAssertion,
		LogExpection,
		LogTitle
	};

	class Logger {
	public:
		Logger(const logging::Logger& logger);
		Logger(const char *file, int line, LogLevel level, bool condition = false);
		~Logger();

		Logger& operator<<(std::wstring& other) {
			mStream << string::narrow(other);
			return *this;
		}

		Logger& operator<<(const std::wstring& other) {
			mStream << string::narrow(other);
			return *this;
		}

		template <typename T, size_t N>
		Logger& operator<<(const T(&other)[N]) {
			mStream << other;
			return *this;
		}

		template <typename T>
		Logger& operator<<(const T& other) {
			mStream << other;
			return *this;
		}

	private:
		bool mCondition{ false };
		LogLevel mLevel;
		std::stringstream mStream;
	};

	void SetLogLevelOn(LogLevel level, bool on);

	void SetLocalLogHandler(std::function<void(std::string_view)> f);

	Logger LogMessage(const char *file, int line, LogLevel level, bool debugBreak = false);
}
}

#define LOG_LEVEL_ON(level) l::logging::SetLogLevelOn(l::logging::LogLevel::level, true)
#define LOG_LEVEL_OFF(level) l::logging::SetLogLevelOn(l::logging::LogLevel::level, false)

#define LOG(level) l::logging::LogMessage(__FILE__, __LINE__, l::logging::LogLevel::level)

#define ASSERT(condition) l::logging::LogMessage(__FILE__, __LINE__, l::logging::LogLevel::LogAssertion, condition)

#define ASSERT_FUZZY(expr1, expr2, tolerance) l::logging::LogMessage(__FILE__, __LINE__, l::logging::LogLevel::LogAssertion, sqrt((expr1 - expr2)*(expr1 - expr2)) < tolerance)

#define EXPECT(condition) l::logging::LogMessage(__FILE__, __LINE__, l::logging::LogLevel::LogExpection, condition)

template<class T>
T* require(T* ptr) {
	if (!(ptr)) { DEBUG_BREAK; } \
	return ptr;
}

#define REQUIRE(ptr) \
	require(ptr);

