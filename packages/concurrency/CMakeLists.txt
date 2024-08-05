cmake_minimum_required (VERSION 3.0.2)

project (concurrency)

set(deps
	logging
	testing
	meta
	memory
)

bs_generate_package(concurrency "tier2" "${deps}" "")
