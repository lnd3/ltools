project (audio)

set(deps
	logging
	testing
	memory
	math
)

bs_generate_package(audio "tier1" "${deps}" "PortAudio")
