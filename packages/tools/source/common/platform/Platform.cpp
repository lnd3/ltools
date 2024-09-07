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