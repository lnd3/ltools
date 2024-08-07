project (physics)

set(deps
	various

	logging
	testing
	tools
)

bs_generate_package(physics "tier3" "${deps}" "")
