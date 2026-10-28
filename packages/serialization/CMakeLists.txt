project (serialization)

set(deps
	logging
	testing

	math
)

bs_generate_package(serialization "tier1" "${deps}" "various;jsonxx")
