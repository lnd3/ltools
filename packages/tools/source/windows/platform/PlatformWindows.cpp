#if defined(BSYSTEM_PLATFORM_Windows)

#include "tools/platform/Platform.h"
#include "logging/Log.h"

#include <shlobj_core.h>

#include <string>
#include <windows.h>
#include <future>

namespace l {
namespace platform {

std::wstring FS::GetSeparator() {
	return L"\\";
}

std::wstring FS::GetProgramPath() {
	constexpr size_t buffer_size = 1024;
	wchar_t wbuffer[buffer_size];
	auto result = SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, wbuffer);
	EXPECT(result == S_OK) << "Failed to get program folder";
	return std::wstring(wbuffer);
}

std::wstring FS::GetAppDataPath() {
	constexpr size_t buffer_size = 1024;
	wchar_t wbuffer[buffer_size];
	auto result = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wbuffer);
	EXPECT(result == S_OK) << "Failed to get app data folder";
	return std::wstring(wbuffer);
}

void Cmd::ParseArguments(int argc, const char* []) {
	std::lock_guard<std::mutex> lock(global::argument_mutex);
	if (global::argument.empty() && global::params.empty()) {
		LPWSTR* argvw2 = CommandLineToArgvW(GetCommandLineW(), &argc);
		global::argument.clear();
		global::params.clear();
		for (int i = 0; i < argc; i++) {
			auto arg = std::wstring(argvw2[i]);
			global::argument.push_back(arg);
			if (i > 0) {
				global::params += arg + (i+1 < argc ? L" ": L"");
			}
		}
		LocalFree(argvw2);
	}
}

bool Proc::IsElevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return fRet;
}

bool Proc::Execute(const std::wstring& file, const std::wstring& args, bool elevated) {
	SHELLEXECUTEINFOW ShExecInfo;

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = NULL;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpFile = file.c_str();
	ShExecInfo.lpParameters = args.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_NORMAL;
	ShExecInfo.hInstApp = NULL;

	if (elevated) {
		ShExecInfo.lpVerb = L"runas";
		LOG(LogInfo) << "Attempt running application with elevated privileges..";
	}

	if (!ShellExecuteExW(&ShExecInfo)) {
		auto err = GetLastError();
		if (err) {
			LOG(LogError) << "Error executing " << file << ". Error: " << err;
		}
		else {
			LOG(LogError) << "Unknown error occured";
		}
		return 1;
	}

	return 0;
}

bool Proc::Fork(std::wstring title, std::wstring& file, const std::wstring& args, bool detach) {

	STARTUPINFOW mStartupInfo;
	PROCESS_INFORMATION mProcessInfo;

	ZeroMemory(&mStartupInfo, sizeof(mStartupInfo));
	mStartupInfo.cb = sizeof(mStartupInfo);
	mStartupInfo.lpTitle = const_cast<LPWSTR>(title.c_str());
	ZeroMemory(&mProcessInfo, sizeof(mProcessInfo));
	mProcessInfo.hThread = 0;

	DWORD dwCreationValues = detach ? CREATE_NEW_CONSOLE : 0;

	auto filename = file.substr(file.find_last_of(L"\\/") + 1);

	auto f = file + std::wstring(L" ") + args;

	// Start the child process. 
	if (!CreateProcessW(NULL,   // No module name (use command line)
		const_cast<LPWSTR>(f.c_str()),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		dwCreationValues,
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&mStartupInfo,            // Pointer to STARTUPINFO structure
		&mProcessInfo)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		LOG(LogError) << "Failed to create process " << filename << ", " << std::to_string(static_cast<long>(GetLastError()));
		return false;
	}

	LOG(LogInfo) << "Successfully started process '" << filename << " " << args << "'";
	if (!detach) {
		LOG(LogInfo) << "Waiting for process exit";
		DWORD waitResult = WaitForSingleObject(mProcessInfo.hProcess, INFINITE);
		if (waitResult == WAIT_FAILED) {
			LOG(LogError) << "Wait for process failed " + std::to_string(static_cast<long>(GetLastError()));
			return false;
		}

		CloseHandle(mProcessInfo.hProcess);
		CloseHandle(mProcessInfo.hThread);
	}

	return true;

}

}
}
#endif