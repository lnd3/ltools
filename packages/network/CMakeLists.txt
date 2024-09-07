project (network)

set(deps
	logging
	testing
	filesystem
	concurrency
)

bs_generate_package(network "tier2" "${deps}" "libcurl;mongoose")
