project (concurrency)

set(deps
	logging
	testing
	meta
	memory
)

bs_generate_package(concurrency "tier1" "${deps}" "")
