cmake_minimum_required (VERSION 3.0.2)

project (memory)

# Version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

set(deps
	logging
	testing

	meta
)

bs_generate_package(memory "tier1" "${deps}" "")
