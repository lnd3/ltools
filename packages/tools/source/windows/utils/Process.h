#pragma once

#include "logging/String.h"

#include <windows.h>
#include <future>


namespace l {
namespace process {

	enum RESULTS {
		SUCCESS = 0,
		FAILED_TO_CREATE_PROCESS,
		FAILED_TO_WAIT_FOR_PROCESS,
		FAILED_TO_DETACH_PROCESS
	};

	class Process {
	private:
		std::wstring mTitle;
		std::wstring mExefile;
		std::wstring mArgs;
		bool mDetach;

		STARTUPINFOW mStartupInfo;
		PROCESS_INFORMATION mProcessInfo;

		std::shared_future<DWORD> processRunner;

	public:
		Process() = delete;
		Process(std::wstring title, std::wstring& exefile, const std::wstring& args, bool detach = false);
		~Process();

		[[nodiscard]] DWORD detach();
		[[nodiscard]] std::shared_future<DWORD> get_future();
	};
}
}