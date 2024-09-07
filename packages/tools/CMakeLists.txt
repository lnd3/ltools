project (tools)

set(deps
	various

	logging
	testing
	memory

	filesystem
	storage
)

bs_generate_package(tools "tier2" "${deps}" "")
