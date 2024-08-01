#pragma once

#include "logging/Log.h"

#include <memory>
#include <sstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <ctime>   // localtime
#include <string>
#include <vector>

#include "logging/String.h"

namespace l {

namespace logging {

	size_t get_time_string(char* buffer, size_t maxSize) {
		using namespace std::chrono;

		auto n = system_clock::now();
		auto tp = n.time_since_epoch();

		auto micros = std::chrono::duration_cast<std::chrono::microseconds>(tp);
		auto seconds = std::chrono::duration_cast<std::chrono::hours>(tp);
		auto minutes = std::chrono::duration_cast<std::chrono::hours>(tp);
		auto hours = std::chrono::duration_cast<std::chrono::hours>(tp);

		time_t now = system_clock::to_time_t(n);
		struct tm newtime;
		localtime_s(&newtime, &now);

		newtime.tm_year += 1900;
		auto micro = static_cast<int>(micros.count() % 1000000);
		auto count = std::snprintf(buffer, maxSize, "%.2d-%.2d-%.2d %.2d:%.2d:%.2d.%.6d", newtime.tm_year, newtime.tm_mon, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec, micro);

		return static_cast<size_t>(count);
	}

	namespace {
		const std::vector<std::string> LogLevelStrings = {
			"    [Debug]",
			"     [Info]",
			"     [Warn]",
			"!   [Error]",
			"     [Test]",
			"!! [Assert]",
			"   [Expect]",
			"###########"
		};
		const std::vector<std::string> LogLevelStrings2 = {
			"Debug",
			"Info",
			"Warn",
			"Error",
			"Test",
			"Assert",
			"Expect",
			"########"
		};

		std::mutex gStreamMutex;
		std::atomic_bool mLogLevelOn[8] = {true, true, true, true, true, true, true, true};

		std::function<void(const std::string&)> LocalLogHandler = [](std::string_view msg) {
			std::cout << msg << std::endl;
		};
	}

	Logger::Logger(const logging::Logger& logger) {
		mLevel = logger.mLevel;
		mStream.str("");
		mStream.clear();
		mStream << logger.mStream.str();
	}

	Logger::Logger(const char *file, int line, LogLevel level, bool condition) : mCondition(condition), mLevel(level), mStream() {
		if (mLevel != LogLevel::LogTitle) {
			std::string fileString = std::string(file);
			size_t indexSlash = fileString.find_last_of("\\/");
			fileString = fileString.substr(indexSlash + 1);

			char buffer[30];
			get_time_string(buffer, sizeof(buffer));
			mStream << "<[" << buffer << "] ";
			mStream << LogLevelStrings2.at(level) << "> ";
			mStream << fileString;
			mStream << "(" << std::to_string(line) << ") ";

			//std::string out = LogLevelStrings.at(level) + " " + get_time_string() + ": " + fileString + "(" + std::to_string(line) + ") ";
			//mStream << out;
		}
		else {
			mStream << LogLevelStrings2.at(level) << " ";
		}
	};
	Logger::~Logger() {
		mStream.flush();
		std::string msg = mStream.str();
		mStream.str("");
		mStream.clear();

		switch (mLevel) {
		case LogLevel::LogDebug:
		case LogLevel::LogInfo:
		case LogLevel::LogWarning:
		case LogLevel::LogError:
		case LogLevel::LogTest:
			break;
		case LogLevel::LogExpection:
			if (!mCondition) {
				// Breaking runtime condition, show popup?
			}
			else {
				msg.clear(); // Error message should not be displayed if condition was right
			}
			break;
		case LogLevel::LogAssertion:
			if (!mCondition) {
				DEBUG_BREAK;
			}
			else {
				msg.clear(); // Error message should not be displayed if condition was right
			}
			break;
		case LogLevel::LogTitle:
			break;
		}

		if (!msg.empty()) {
			if (mLogLevelOn[mLevel]) {
				std::lock_guard<std::mutex> lock(gStreamMutex);
				LocalLogHandler(msg);
			}
		}
	}

	void SetLogLevelOn(LogLevel level, bool on) {
		mLogLevelOn[level] = on;
	}

	void SetLocalLogHandler(std::function<void(std::string_view)> f) {
		std::lock_guard<std::mutex> lock(gStreamMutex);
		LocalLogHandler = f;
	}

	Logger LogMessage(const char *file, int line, LogLevel level, bool condition) {
		return l::logging::Logger::Logger(file, line, level, condition);
	}

}
}

