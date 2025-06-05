project (rendering)

# Version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

set(deps
	logging

	testing
	memory
	math
	audio
	hid
	nodegraph

	filesystem
	tools

	physics
)

set(deps_external
	jsonxx
	various
	pugixml
	openfbx
	ufbx
	bc7enc
	PortAudio
)

if(TARGET glfw)
	list(APPEND deps_external glfw glad)
endif()

if(TARGET imgui)
	list(APPEND deps_external imgui implot)
endif()

find_package(Vulkan)
if(TARGET Vulkan::Vulkan)
	list(APPEND deps Vulkan::Vulkan)
	list(APPEND deps_external "${Vulkan_INCLUDE_DIRS}")
endif()

bs_generate_package(rendering "tier3" "${deps}" "${deps_external}")
