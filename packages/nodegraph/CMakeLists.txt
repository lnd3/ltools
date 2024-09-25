project (nodegraph)

set(deps
	various

	logging

	testing
	memory
	math
	audio
	hid
)

bs_generate_package(nodegraph "tier1" "${deps}" "PortAudio")
