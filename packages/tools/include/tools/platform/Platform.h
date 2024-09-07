#pragma once

#include <string>
#include <vector>
#include <mutex>

namespace l {
namespace platform {
	namespace global {
		extern std::mutex argument_mutex;
		extern std::vector<std::wstring> argument;
		extern std::wstring params;
	}

enum class platform {
		WINDOWS = 0,
		UWP,
		LINUX,
		ANDROID,
		CYGWIN,
		MSYS,
		GNU,
		OSX,
		IOS,
		UNKNOWN
	};

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

	class FS {
	public:
		static std::wstring GetSeparator();
		static std::wstring GetProgramPath();
		static std::wstring GetAppDataPath();
	};

	class Cmd {
	public:
		static void ParseArguments(int argc, const char* argv[]);
		static std::wstring GetCommandLineArgument(size_t index);
		static std::wstring GetCommandLineString();
		static size_t GetCommandLineCount();
	};

	class Proc {
	public:
		static bool IsElevated();
		static bool Execute(const std::wstring& file, const std::wstring& params, bool elevated = false);
		static bool Fork(std::wstring title, std::wstring& exefile, const std::wstring& args, bool detach = false);
	};
}
}
