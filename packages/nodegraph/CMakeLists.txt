project (nodegraph)

set(deps
	logging
	testing
)

bs_generate_package(nodegraph "tier1" "${deps}" "")
