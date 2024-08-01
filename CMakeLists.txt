cmake_minimum_required (VERSION 3.0.2)

project(ltools)

include(CTest)
enable_testing()

add_subdirectory(deps/ldeps)

include(bs)

bs_project()

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


bs_setup("packages" "${PACKAGE_NAMES}")

add_subdirectory(apps)


