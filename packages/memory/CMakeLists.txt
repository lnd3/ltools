cmake_minimum_required (VERSION 3.0.2)

project (memory)

set(deps
	logging
	testing

	meta
)

bs_generate_package(memory "tier1" "${deps}" "")
