project (nn)

set(deps
	logging
	testing
	memory
	serialization

	filesystem
	storage
)

set(deps_external
	floatfann_static
	ncnn
	tiny_dnn_static
)

bs_generate_package(nn "tier1" "${deps}" "${deps_external}")
