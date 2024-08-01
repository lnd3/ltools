cmake_minimum_required (VERSION 3.0.2)

project (rendering)

# Version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

set(deps
	logging

	testing

	filesystem
	memory

	physics

	various
	pugixml
	openfbx
	ufbx
	bc7enc
)

set(deps_includes)

if(TARGET glfw)
	list(APPEND deps glfw glad)
endif()

if(TARGET imgui)
	list(APPEND deps imgui implot)
endif()

find_package(Vulkan)
if(TARGET Vulkan::Vulkan)
	list(APPEND deps Vulkan::Vulkan)
	list(APPEND deps_includes "${Vulkan_INCLUDE_DIRS}")
endif()

bs_generate_package(rendering "tier3" "${deps}" "${deps_includes}")
