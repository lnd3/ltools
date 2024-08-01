cmake_minimum_required (VERSION 3.0.2)

project (filesystem)

set(deps
	logging

	testing
)

bs_generate_package(filesystem "tier2" "${deps}" "")
