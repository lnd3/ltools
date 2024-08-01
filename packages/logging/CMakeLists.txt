cmake_minimum_required (VERSION 3.0.2)

project (logging)

set(deps
	""
)

bs_generate_package(logging "tier0" "${deps}" "")
