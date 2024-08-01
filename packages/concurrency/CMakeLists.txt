cmake_minimum_required (VERSION 3.0.2)

project (concurrency)

# Version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

set(deps
	logging
	testing
	meta
	memory
)

bs_generate_package(concurrency "tier2" "${deps}" "")
