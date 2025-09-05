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
target_compile_definitions(nn PUBLIC cxx_std_17)
if(MSVC)
	target_compile_options(nn PRIVATE /W2 /bigobj)
else()
	target_compile_options(nn PRIVATE -Werror -Wno-unused-result)
endif()
