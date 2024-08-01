cmake_minimum_required (VERSION 3.0.2)

project (meta)

# Version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

set(deps
	logging

	testing
)

bs_generate_package(meta "tier1" "${deps}" "")
