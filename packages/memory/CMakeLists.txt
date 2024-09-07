project (memory)

set(deps
	logging
	testing

	meta
)

bs_generate_package(memory "tier1" "${deps}" "")
