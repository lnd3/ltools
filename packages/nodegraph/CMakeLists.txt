project (nodegraph)

set(deps
	logging

	testing
	memory
	math
	serialization
	audio
	hid
)

bs_generate_package(nodegraph "tier1" "${deps}" "PortAudio;various")
