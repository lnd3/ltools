#if defined(BSYSTEM_PLATFORM_Linux)

#include "tools/platform/Platform.h"
#include "logging/Log.h"

#include <string>
#include <future>

namespace l {
namespace platform {

std::wstring FS::GetSeparator() {
	return L"/";
}

std::wstring FS::GetProgramPath() {
	constexpr size_t buffer_size = 1024;
	wchar_t wbuffer[buffer_size];
	return std::wstring(wbuffer);
}

std::wstring FS::GetAppDataPath() {
	constexpr size_t buffer_size = 1024;
	wchar_t wbuffer[buffer_size];
	return std::wstring(wbuffer);
}

void Cmd::ParseArguments(int argc, const char* argv[]) {
	std::lock_guard<std::mutex> lock(global::argument_mutex);
	if (global::argument.empty() && global::params.empty()) {
		global::argument.clear();
		global::params.clear();
		for (int i = 0; i < argc; i++) {
			auto arg = std::wstring(argv[i]);
			global::argument.push_back(arg);
			if (i > 0) {
				global::params += arg + (i+1 < argc ? L" ": L"");
			}
		}
	}
}

bool Proc::IsElevated() {
	return false;
}

bool Proc::Execute(const std::wstring&, const std::wstring&, bool) {
	return 0;
}

bool Proc::Fork(std::wstring, std::wstring&, const std::wstring&, bool) {
	return false;

}

}
}
#endif