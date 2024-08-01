cmake_minimum_required (VERSION 3.0.2)

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
