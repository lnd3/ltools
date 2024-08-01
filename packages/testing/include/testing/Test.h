#pragma once

#include <math.h>
#include <string>
#include <map>
#include <chrono>

#include "logging/Log.h"
#include "logging/Static.h"
#include "logging/Macro.h"

#include "testing/Timer.h"

namespace l {
namespace testing {
    int add_test(const std::string& groupName, const std::string& testName, std::function<int(void)> f);
    int add_perf(const std::string& groupName, const std::string& perfName, std::function<int(void)> f);

    bool run_tests(const char* app);
    bool run_perfs(const char* app);
}
}

#define ADDTEST(testgroup, testname, function) static int UNIQUE(block) = l::testing::add_test(testgroup, testname, function); \

#define TESTNAME(group, name) CONCATE(Test, CONCATE(group, name)) \

#define TEST(testgroup, testname) \
	int TESTNAME(testgroup, testname)(); \
	ADDTEST(STRINGIFY(testgroup), STRINGIFY(testname), TESTNAME(testgroup, testname)) \
	int TESTNAME(testgroup, testname)() \



#define ADDPERF(perfgroup, perfname, function) static int UNIQUE(block) = l::testing::add_perf(perfgroup, perfname, function); \

#define PERFNAME(group, name) CONCATE(Perf, CONCATE(group, name)) \

#define PERF_TEST(perfgroup, perfname) \
	int PERFNAME(perfgroup, perfname)(); \
	int PERFNAME(perfgroup, perfname)Wrapper() {l::testing::get_current_test_group()=#perfgroup;return PERFNAME(perfgroup, perfname)();} \
	ADDPERF(STRINGIFY(perfgroup), STRINGIFY(perfname), PERFNAME(perfgroup, perfname)Wrapper) \
	int PERFNAME(perfgroup, perfname)() \

#ifndef _DEBUG
#define TEST_TRUE(expr, msg) \
    if (!(expr)) { LOG(LogError) << msg; LOG(LogTest) << "Test failed"; return 1;} \

#define TEST_FALSE(expr, msg) \
    if (!!(expr)) { LOG(LogError) << msg; LOG(LogTest) << "Test failed"; return 1;} \

#define TEST_EQ(expr1, expr2, msg) \
    if (expr1 != expr2) { LOG(LogError) << msg; LOG(LogTest) << "Test failed"; return 1;} \

#define TEST_FUZZY(expr1, expr2, tolerance, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > tolerance) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ")" << msg; LOG(LogTest) << "Test failed: "; return 1;} \

#define TEST_FUZZY2(expr1, expr2, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > 0.000000001) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ")" << msg; LOG(LogTest) << "Test failed: "; return 1;} \

#define TEST_TRUE_NO_RET(expr, msg) \
    if (!(expr)) { LOG(LogError) << msg; LOG(LogTest) << "Test failed";} \

#define TEST_FALSE_NO_RET(expr, msg) \
    if (!!(expr)) { LOG(LogError) << msg; LOG(LogTest) << "Test failed";} \

#define TEST_EQ_NO_RET(expr1, expr2, msg) \
    if (expr1 != expr2) { LOG(LogError) << msg; LOG(LogTest) << "Test failed";} \

#define TEST_FUZZY_NO_RET(expr1, expr2, tolerance, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > tolerance) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ")" << msg; LOG(LogTest) << "Test failed: ";} \

#define TEST_FUZZY2_NO_RET(expr1, expr2, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > 0.000000001) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ")" << msg; LOG(LogTest) << "Test failed: ";} \


#else
#define TEST_TRUE(expr, msg) \
    if (!(expr)) { LOG(LogError) << "TEST_TRUE(" << std::to_string(expr) << ") " << msg; LOG(LogTest) << "Test failed"; DEBUG_BREAK;return 1;} \

#define TEST_FALSE(expr, msg) \
    if (!!(expr)) { LOG(LogError) << "TEST_FALSE(" << std::to_string(expr) << ") " << msg; LOG(LogTest) << "Test failed"; DEBUG_BREAK;return 1;} \

#define TEST_EQ(expr1, expr2, msg) \
    if (expr1 != expr2) { LOG(LogError) << "TEST_EQ(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ") " << msg; LOG(LogTest) << "Test failed"; DEBUG_BREAK;return 1;} \

#define TEST_FUZZY(expr1, expr2, tolerance, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > tolerance) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ") " << msg; LOG(LogTest) << "Test failed: "; DEBUG_BREAK;return 1;} \

#define TEST_FUZZY2(expr1, expr2, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > 0.000000001) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ") " << msg; LOG(LogTest) << "Test failed: "; DEBUG_BREAK;return 1;} \

#define TEST_TRUE_NO_RET(expr, msg) \
    if (!(expr)) { LOG(LogError) << "TEST_TRUE(" << std::to_string(expr) << ") " << msg; LOG(LogTest) << "Test failed"; DEBUG_BREAK;} \

#define TEST_FALSE_NO_RET(expr, msg) \
    if (!!(expr)) { LOG(LogError) << "TEST_FALSE(" << std::to_string(expr) << ") " << msg; LOG(LogTest) << "Test failed"; DEBUG_BREAK;} \

#define TEST_EQ_NO_RET(expr1, expr2, msg) \
    if (expr1 != expr2) { LOG(LogError) << "TEST_EQ(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ") " << msg; LOG(LogTest) << "Test failed"; DEBUG_BREAK;} \

#define TEST_FUZZY_NO_RET(expr1, expr2, tolerance, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > tolerance) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ") " << msg; LOG(LogTest) << "Test failed: "; DEBUG_BREAK;} \

#define TEST_FUZZY2_NO_RET(expr1, expr2, msg) \
    if (sqrt((expr1 - expr2)*(expr1 - expr2)) > 0.000000001) { LOG(LogError) << "TEST_FUZZY(" << std::to_string(expr1) << ", " << std::to_string(expr2) << ") " << msg; LOG(LogTest) << "Test failed: "; DEBUG_BREAK;} \


#endif

#define TEST_RUN(app) \
	if (!l::testing::run_tests(app)) { LOG(LogTest) << "Test run failed"; return 1;} \
	if (!l::testing::run_perfs(app)) { LOG(LogTest) << "Perf run failed"; return 1;} \
