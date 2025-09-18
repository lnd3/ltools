project (nn)

set(deps
	logging
	testing
	memory
	serialization

	filesystem
)

set(deps_external
	floatfann_static
	ncnn
	tiny_dnn_static
)

if(TARGET libtorch)
	list(APPEND deps_external
		libtorch
	)
endif()

bs_generate_package(nn "tier1" "${deps}" "${deps_external}")
set_target_properties(nn PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
set_target_properties(nn_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

if(TARGET libtorch)
	bs_copy_shared_libs_dir(nn_test "${LDEPS_LIBTORCH_LIB_DIR}")
endif()

if(MSVC)
	target_compile_options(nn PRIVATE /WX- /bigobj)
else()
	target_compile_options(nn PRIVATE -Werror -Wno-unused-result -Wno-ignored-qualifiers -Wno-unused-parameter)
endif()
