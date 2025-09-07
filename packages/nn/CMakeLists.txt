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

bs_generate_package(nn "tier1" "${deps}" "${deps_external}")
set_target_properties(nn PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
set_target_properties(nn_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

if(MSVC)
	target_compile_options(nn PRIVATE /W2 /bigobj)
else()
	target_compile_options(nn PRIVATE -Werror -Wno-unused-result -Wno-ignored-qualifiers -Wno-unused-parameter)
endif()
