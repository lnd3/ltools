project (nodegraph)

set(deps
	various

	logging

	testing
	memory
	hid
	audio
)

bs_generate_package(nodegraph "tier1" "${deps}" "")
