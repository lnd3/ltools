project (network)

set(deps
	libcurl
	mongoose

	logging
	testing
	filesystem
	concurrency
)

bs_generate_package(network "tier2" "${deps}" "")
