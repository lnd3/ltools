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
		filesystem
		meta
		memory
		math
		concurrency
		serialization
		crypto
		audio
		hid
		nodegraph
		nn

		network
		storage
		tools

		physics
		ecs
		rendering
	)


	bs_configure_packages("packages" "${PACKAGE_NAMES}")

	option(LTOOLS_ENABLE_UNITY_BUILD "Enable unity builds for faster compilation" OFF)
	if(LTOOLS_ENABLE_UNITY_BUILD)
	  set_target_properties(logging PROPERTIES UNITY_BUILD ON)

	  set_target_properties(testing PROPERTIES UNITY_BUILD ON)
	  set_target_properties(filesystem PROPERTIES UNITY_BUILD ON)
	  set_target_properties(meta PROPERTIES UNITY_BUILD ON)
	  set_target_properties(memory PROPERTIES UNITY_BUILD ON)
	  set_target_properties(concurrency PROPERTIES UNITY_BUILD ON)
	  set_target_properties(serialization PROPERTIES UNITY_BUILD ON)
	  set_target_properties(crypto PROPERTIES UNITY_BUILD ON)
	  set_target_properties(math PROPERTIES UNITY_BUILD ON)
	  set_target_properties(audio PROPERTIES UNITY_BUILD ON)
	  set_target_properties(hid PROPERTIES UNITY_BUILD ON)
	  set_target_properties(nodegraph PROPERTIES UNITY_BUILD ON)
	  set_target_properties(nn PROPERTIES UNITY_BUILD ON)

	  set_target_properties(network PROPERTIES UNITY_BUILD ON)
	  set_target_properties(storage PROPERTIES UNITY_BUILD ON)
	  set_target_properties(tools PROPERTIES UNITY_BUILD ON)

	  set_target_properties(physics PROPERTIES UNITY_BUILD ON)
	  set_target_properties(ecs PROPERTIES UNITY_BUILD ON)
	  #set_target_properties(rendering PROPERTIES UNITY_BUILD ON)

	  set_target_properties(testing_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(filesystem_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(meta_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(memory_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(concurrency_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(serialization_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(crypto_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(math_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(audio_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(hid_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(nodegraph_test PROPERTIES UNITY_BUILD ON)

	  set_target_properties(network_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(storage_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(tools_test PROPERTIES UNITY_BUILD ON)

	  set_target_properties(physics_test PROPERTIES UNITY_BUILD ON)
	  #set_target_properties(ecs_test PROPERTIES UNITY_BUILD ON)
	  set_target_properties(rendering_test PROPERTIES UNITY_BUILD ON)

	endif()

endif()
