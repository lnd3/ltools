#include <logging/Log.h>

#include <memory>
#include <sstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <string>
#include <vector>
#include <time.h>
#include <atomic>

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

		const time_t now = system_clock::to_time_t(n);
		struct tm newtime;
#ifdef WIN32
		localtime_s(&newtime, &now);
#else
		localtime_r(&now, &newtime);
#endif
		newtime.tm_year += 1900;
		auto micro = static_cast<int>(micros.count() % 1000000);

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		auto count = std::snprintf(buffer, maxSize, "%.2d-%.2d-%.2d %.2d:%.2d:%.2d.%.6d", newtime.tm_year, newtime.tm_mon, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec, micro);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
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
		std::atomic<bool> mLogLevelOn[8] = { true, true, true, true, true, true, true, true };

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

	std::string_view rcut(std::string_view s, const char ch) {
		auto i = s.rfind(ch) + 1;
		if (i != std::string::npos) {
			return std::string_view(s.data() + i, s.size() - i);
		}
		return s;
	}

	Logger::Logger(const char *file, int line, LogLevel level, bool condition) : mCondition(condition), mLevel(level), mStream() {
		if (mLevel != LogLevel::LogTitle) {
			auto fileView = std::string_view(file);
			auto ch = fileView.data() + fileView.size() - 1;
			size_t i;
			for (i = 0;i<fileView.size();i++) {
				if (*ch == '\\' || *ch == '/') {
					ch++;
					break;
				}
				ch--;
			}
			auto filename = std::string_view(ch, i);

			char buffer[30];
			auto size = get_time_string(buffer, sizeof(buffer));
			auto timeview = std::string_view(buffer, size);
			mStream << "<[";
			mStream << timeview;
			mStream << "] ";
			mStream << LogLevelStrings2.at(level);
			mStream << "> ";
			mStream << filename;
			mStream << "(";
			mStream << line;
			mStream << ") ";

			//std::string out = LogLevelStrings.at(level) + " " + get_time_string() + ": " + fileString + "(" + std::to_string(line) + ") ";
			//mStream << out;
		}
		else {
			mStream << LogLevelStrings2.at(level) << " ";
		}
	};
	Logger::~Logger() {
		if (mCondition && (mLevel == LogLevel::LogExpection || mLevel == LogLevel::LogAssertion)) {
			return;
		}

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
		case LogLevel::LogTitle:
			break;
		case LogLevel::LogExpection:
			if (!mCondition) {
				// Breaking runtime condition, show popup?
			}
			// already handled in early return
			//else {
			//	msg.clear(); // Error message should not be displayed if condition was right
			//}
			break;
		case LogLevel::LogAssertion:
			if (!mCondition) {
				DEBUG_BREAK;
			}
			// already handled in early return
			//else {
			//	msg.clear(); // Error message should not be displayed if condition was right
			//}
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

	Logger LogMessage(const char *file, int line, l::logging::LogLevel level, bool condition) {
		return l::logging::Logger(file, line, level, condition);
	}

}
}

