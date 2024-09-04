cmake_minimum_required (VERSION 3.21.7)

project(ltools)

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
	message("'ltools' expect 'ldeps' has been externally added to the parent project")
	file(REMOVE_RECURSE deps)
else()
	include(CTest)
	enable_testing()

	message("'ltools' expect 'ldeps' be present in 'deps/ldeps'")
	add_subdirectory(deps/ldeps)

	include(bs)
	bs_init()

	# define all package libraries to use
	set(PACKAGE_NAMES
		logging

		testing
		meta
		memory
		math
		hid
		audio
		nodegraph

		concurrency
		filesystem
		network
		storage
		tools

		physics
		ecs
		rendering
	)


	bs_configure_packages("packages" "${PACKAGE_NAMES}")
endif()
