cmake_minimum_required (VERSION 3.0.2)

project (meta)

set(deps
	logging
	testing
)

bs_generate_package(meta "tier1" "${deps}" "")
