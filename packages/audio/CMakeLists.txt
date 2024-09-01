project (audio)

set(deps
	various

	logging
	testing
	memory
)

bs_generate_package(audio "tier1" "${deps}" "")
