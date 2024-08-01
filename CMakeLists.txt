cmake_minimum_required (VERSION 3.0.2)

project(ltools)

include(CTest)
enable_testing()

#add_subdirectory(deps)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/deps/bs/cmake)
include(bs)

bs_project(1 1)

# define all package libraries to use
set(PACKAGE_NAMES
	logging

	testing
	meta
	memory

	concurrency
	filesystem
	network
	storage
	tools

	physics
	ecs
	rendering
)


bs_setup("deps/ltools" "${PACKAGE_NAMES}")

add_subdirectory(apps)


