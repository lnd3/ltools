project (serialization)

set(deps
	various
	logging
	testing
)

bs_generate_package(serialization "tier1" "${deps}" "")
