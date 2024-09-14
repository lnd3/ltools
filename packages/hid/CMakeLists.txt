project (hid)

set(deps
	logging
	testing

	math
	audio
)

bs_generate_package(hid "tier1" "${deps}" "")
