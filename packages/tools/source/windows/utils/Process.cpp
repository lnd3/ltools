#pragma once

#include "Process.h"

#include "logging/Log.h"

#include <string>

#include <windows.h>
#include <future>

namespace l {
namespace process {

	Process::Process(std::wstring title,
		std::wstring& exefile,
		const std::wstring& args,
		bool detach)
		: mTitle(title)
		, mExefile(exefile)
		, mArgs(args)
		, mDetach(detach) {
		processRunner = std::async(std::launch::deferred, [this]() -> DWORD {
			
			ZeroMemory(&mStartupInfo, sizeof(mStartupInfo));
			mStartupInfo.cb = sizeof(mStartupInfo);
			mStartupInfo.lpTitle = const_cast<LPWSTR>(mTitle.c_str());
			ZeroMemory(&mProcessInfo, sizeof(mProcessInfo));

			DWORD dwCreationValues = mDetach ? CREATE_NEW_CONSOLE : 0;

			auto filename = mExefile.substr(mExefile.find_last_of(L"\\/") + 1);

			auto f = mExefile + std::wstring(L" ") + mArgs;

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
				return FAILED_TO_CREATE_PROCESS;
			}

			if (!mDetach) {
				LOG(LogInfo) << "Successfully started process " << filename << " " << mArgs;
				// Wait until child process exits.
				DWORD waitResult = WaitForSingleObject(mProcessInfo.hProcess, INFINITE);
				if (waitResult == WAIT_FAILED) {
					LOG(LogError) << "Wait for process failed " + std::to_string(static_cast<long>(GetLastError()));
					return FAILED_TO_WAIT_FOR_PROCESS;
				}
				else {
					LOG(LogInfo) << "Wait ended for process " << filename;
				}
			}

			return SUCCESS;
		});
	}

	Process::~Process() {
		if (processRunner.valid()) {
			processRunner.wait();
		}
		if (mProcessInfo.hProcess) {
			CloseHandle(mProcessInfo.hProcess);
		}
		if (mProcessInfo.hThread) {
			CloseHandle(mProcessInfo.hThread);
		}
	}

	[[nodiscard]] DWORD Process::detach() {
		if (processRunner.valid()) {
			mDetach = true;
			processRunner.wait();
			return processRunner.get();
		}
		else {
			LOG(LogError) << "Failed to detach process";
			return FAILED_TO_DETACH_PROCESS;
		}
	}

	[[nodiscard]] std::shared_future<DWORD> Process::get_future() {
		return processRunner;
	}
}
}
