cmake_minimum_required (VERSION 3.0.2)

project (testing)

set(deps
	logging
)

bs_generate_package(testing "tier1" "${deps}" "")
