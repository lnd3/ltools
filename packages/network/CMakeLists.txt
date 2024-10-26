project (network)

set(deps
	logging
	testing

	serialization

	filesystem
	concurrency
)

bs_generate_package(network "tier2" "${deps}" "libcurl;mongoose")
