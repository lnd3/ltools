project (nodegraph)

set(deps
	logging

	testing
	filesystem
	memory
	math
	serialization
	audio
	hid
)

bs_generate_package(nodegraph "tier1" "${deps}" "PortAudio;various;jsonxx")
