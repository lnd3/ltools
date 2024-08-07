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

	bool IsPlatformPosixCompatible();
	std::string_view GetPlatformName();
	std::string_view GetPlatformVersion();
	platform GetPlatform();

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
