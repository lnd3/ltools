project (physics)

set(deps
	various

	logging
	testing
	math
	tools
)

bs_generate_package(physics "tier3" "${deps}" "")
