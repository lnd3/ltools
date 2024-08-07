#include "Process.h"

#include "logging/Log.h"

#include <string>
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
		processRunner = std::async(std::launch::deferred, [this]() -> uint32_t {
			return FAILED_TO_CREATE_PROCESS;
		});
	}

	Process::~Process() {
		if (processRunner.valid()) {
			processRunner.wait();
		}
	}

	[[nodiscard]] uint32_t Process::detach() {
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

	[[nodiscard]] std::shared_future<uint32_t> Process::get_future() {
		return processRunner;
	}
}
}
