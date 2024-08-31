project (nodegraph)

set(deps
	logging
	testing
	hid
)

bs_generate_package(nodegraph "tier1" "${deps}" "")
