project (ecs)

set(deps
	logging
	testing
	memory

	tools
)

bs_generate_package(ecs "tier3" "${deps}" "")
