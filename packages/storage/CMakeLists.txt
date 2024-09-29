project (storage)

set(deps
	logging
	testing
	serialization

	filesystem
	memory
	concurrency
)

bs_generate_package(storage "tier2" "${deps}" "various")
