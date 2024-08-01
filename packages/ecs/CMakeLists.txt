cmake_minimum_required (VERSION 3.0.2)

project (ecs)

set(deps
	logging
	testing
	memory

	tools
)

bs_generate_package(ecs "tier3" "${deps}" "")
