#include "tools/platform/Platform.h"

namespace l {
namespace platform {
	namespace global {
		std::mutex argument_mutex;
		std::vector<std::wstring> argument;
		std::wstring params;
	}

	platform GetPlatform() {
#if defined(WIN32) || defined(_WIN32)
		return platform::WIN;
#elif defined(__linux__)
		return platform::LINUX;
#elif defined(CYGWIN)
		return platform::ANDROID;
#elif defined(APPLE)
		return platform::APPLE;
#elif defined(LINUX)
		return platform::APPLE;
#else
		return platform::UNKNOWN;
#endif
	}

	std::wstring Cmd::GetCommandLineArgument(size_t index) {
		std::lock_guard<std::mutex> lock(global::argument_mutex);
		if (index >= global::argument.size()) {
			return L"";
		}
		return global::argument.at(index);
	}

	std::wstring Cmd::GetCommandLineString() {
		std::lock_guard<std::mutex> lock(global::argument_mutex);
		return global::params;
	}

	size_t Cmd::GetCommandLineCount() {
		std::lock_guard<std::mutex> lock(global::argument_mutex);
		return global::argument.size();
	}



}
}