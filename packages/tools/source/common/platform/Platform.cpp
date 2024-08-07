#include "tools/platform/Platform.h"

// Useful platform define article
// https://stackoverflow.com/questions/2989810/which-cross-platform-preprocessor-defines-win32-or-win32-or-win32
// Cmake supported system platforms
// https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_NAME.html

namespace l {
namespace platform {
	namespace global {
		std::mutex argument_mutex;
		std::vector<std::wstring> argument;
		std::wstring params;
	}

	bool constexpr IsPlatformPosixCompatible() {
#if defined(BSYSTEM_PLATFORM_Linux) || defined(BSYSTEM_PLATFORM_Android) || defined(BSYSTEM_PLATFORM_CYGWIN) || defined(BSYSTEM_PLATFORM_MSYS) || defined(BSYSTEM_PLATFORM_GNU) || defined(BSYSTEM_PLATFORM_iOS) || defined(BSYSTEM_PLATFORM_Darwin)
		return true;
#endif
		return false;
	}

	std::string_view constexpr GetPlatformName() {
		return BSYSTEM_PLATFORM;
	}

	std::string_view constexpr GetPlatformVersion() {
		return BSYSTEM_VERSION;
	}

	platform constexpr GetPlatform() {
#if defined(BSYSTEM_PLATFORM_Windows)
		return platform::WINDOWS;
#elif defined(BSYSTEM_PLATFORM_WindowsStore)
		return platform::UWP;

#elif defined(BSYSTEM_PLATFORM_Linux)
		return platform::LINUX;
#elif defined(BSYSTEM_PLATFORM_Android)
		return platform::ANDROID;
#elif defined(BSYSTEM_PLATFORM_CYGWIN)
		return platform::CYGWIN;
#elif defined(BSYSTEM_PLATFORM_MSYS)
		return platform::MSYS;
#elif defined(BSYSTEM_PLATFORM_GNU)
		return platform::GNU;

#elif defined(BSYSTEM_PLATFORM_iOS)
		return platform::IOS;
#elif defined(BSYSTEM_PLATFORM_Darwin)
		return platform::OSX;
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