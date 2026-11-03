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

bs_generate_package(nodegraph "tier1" "${deps}" "various;jsonxx")
if(TARGET PortAudio)
    target_link_libraries(nodegraph PUBLIC PortAudio)
endif()
