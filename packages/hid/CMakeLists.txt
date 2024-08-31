project (hid)

set(deps
	logging
	testing
)

bs_generate_package(hid "tier1" "${deps}" "")
