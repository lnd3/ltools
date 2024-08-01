cmake_minimum_required (VERSION 3.0.2)

project (storage)

set(deps
	various
	logging
	testing
	filesystem
	memory
	concurrency
)

bs_generate_package(storage "tier2" "${deps}" "")
