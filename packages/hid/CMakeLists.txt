project (hid)

set(deps
	logging
	testing

	math
)

bs_generate_package(hid "tier1" "${deps}" "")
