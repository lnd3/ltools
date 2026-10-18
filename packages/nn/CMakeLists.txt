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

if(TARGET libtorch_wrapper)
	list(APPEND deps_external
		libtorch_wrapper
	)
endif()

bs_generate_package(nn "tier1" "${deps}" "${deps_external}")
set_target_properties(nn PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
set_target_properties(nn_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

if(TARGET libtorch_wrapper)
	message("libtorch: ${LDEPS_LIBTORCH_LIB_DIR}")
	message("cuda: ${CUDA_TOOLKIT_ROOT_DIR}")

	target_compile_definitions(nn PUBLIC HAS_LIBTORCH=1)
	target_compile_definitions(nn_test PUBLIC HAS_LIBTORCH=1)

	if(WIN32)
		bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/c10")
		bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/torch")
		bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/torch_cpu")
		if(USE_LIBTORCH_CUDA)
			bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/c10_cuda")
			bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/torch_cuda")
			bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/libiomp5md")
			bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/cupti64_2025.3.0")
		endif()
	else()
		bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/libc10")
		bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/libtorch")
		bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/libtorch_cpu")
		if(USE_LIBTORCH_CUDA)
			bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/libc10_cuda")
			bs_copy_shared_lib(nn_test "${LDEPS_LIBTORCH_LIB_DIR}/libtorch_cuda")
		endif()
	endif()
endif()

if(MSVC)
	target_compile_options(nn PRIVATE /WX- /bigobj)
	target_compile_options(nn_test PRIVATE /WX- /bigobj)
else()
	target_compile_options(nn PRIVATE -Wno-unused-variable -Wno-error -Wno-unused-variable -Wno-unused-result -Wno-ignored-qualifiers -Wno-unused-parameter)
	target_compile_options(nn_test PRIVATE -Wno-unused-variable -Wno-error -Wno-unused-variable -Wno-unused-result -Wno-ignored-qualifiers -Wno-unused-parameter)
endif()
