project (filesystem)

set(deps
	logging
	testing
)

bs_generate_package(filesystem "tier1" "${deps}" "")
