#pragma once

#include <map>
#include <vector>
#include <functional>

#include "logging/Log.h"
#include "testing/Test.h"
#include "testing/Timer.h"

namespace l {
namespace testing {

	using TestItem = std::vector<std::pair<std::string, std::function<int(void)>>>;

	auto& get_test_groups() {
		static std::map<std::string, std::unique_ptr<TestItem>> test_groups;
		return test_groups;
	}

	TestItem& get_test_group(const std::string& groupName) {
		auto& groups = get_test_groups();
		auto it = groups.find(groupName);
		if (it != groups.end()) {
			return *it->second;
		}
		groups.emplace(groupName, std::make_unique<TestItem>());
		it = groups.find(groupName);
		return *it->second;
	}

	int add_test(const std::string& groupName, const std::string& testName, std::function<int(void)> f) {
		get_test_group(groupName).push_back({ testName, std::move(f) });
		return 0;
	}

	using PerfItem = std::vector<std::pair<std::string, std::function<int(void)>>>;

	auto& get_perf_groups() {
		static std::map<std::string, std::unique_ptr<PerfItem>> perf_groups;
		return perf_groups;
	}

	PerfItem& get_perf_group(const std::string& groupName) {
		auto& groups = get_perf_groups();
		auto it = groups.find(groupName);
		if (it != groups.end()) {
			return *it->second;
		}
		groups.emplace(groupName, std::make_unique<PerfItem>());
		it = groups.find(groupName);
		return *it->second;
	}

	int add_perf(const std::string& groupName, const std::string& perfName, std::function<int(void)> f) {
		get_perf_group(groupName).push_back({ perfName, std::move(f) });
		return 0;
	}

	bool run_tests(const char* app) {
		LOG(LogTitle) << "Unit tests " << app;

		bool test_success = true;
		{
			size_t failed_tests = 0;

			std::vector<std::string> summary;

			auto& groups = get_test_groups();
			for (auto& groupIt : groups) {
				size_t total = groupIt.second->size();
				LOG(LogTitle) << "# " << groupIt.first;
				for (auto& f : *groupIt.second) {
					LOG(LogTitle) << "## " << groupIt.first + "::" + f.first;
					if (f.second()) {
						failed_tests++;
						test_success = false;
					}
				}
				std::ostringstream msg;
				msg << "## Test result for '" + groupIt.first + "': successful tests(" << (total - failed_tests) << " / " << (total) << ")";
				LOG(LogTitle) << msg.str();
				summary.push_back(msg.str());
			}

			LOG(LogTitle) << "## Test summary ";
			for (auto& str : summary) {
				LOG(LogTitle) << str;
			}

			if (!test_success) {
				LOG(LogTitle) << "All tests did not go through successfully..";
			}
		}

		return test_success;
	}

	bool run_perfs(const char* app) {
		LOG(LogTitle) << "Performance tests " << app;

		bool perf_success = true;
		{
			size_t failed_perfs = 0;

			std::vector<std::string> summary;

			auto& groups = get_perf_groups();
			for (auto& groupIt : groups) {
				size_t total = groupIt.second->size();
				LOG(LogTitle) << "## " << groupIt.first;
				for (auto& f : *groupIt.second) {
					LOG(LogTitle) << groupIt.first + "::" + f.first;
					if (f.second()) {
						failed_perfs++;
						perf_success = false;
					}

					auto& measures = get_time_measures(groupIt.first);

					for (auto& result : measures) {
						LOG(LogInfo) << groupIt.first << "::" << result.first << ": " << result.second.mSeconds << " sec.";
					}
				}

				std::ostringstream msg;
				msg << "Performance result for '" + groupIt.first + "': successful perfs(" << (total - failed_perfs) << " / " << (total) << ")";
				LOG(LogTitle) << msg.str();
				summary.push_back(msg.str());
			}

			LOG(LogTitle) << "----";
			for (auto& str : summary) {
				LOG(LogTitle) << str;
			}

			if (!perf_success) {
				LOG(LogTitle) << "All perfs did not go through successfully..";
			}
		}

		return perf_success;
	}
}
}
