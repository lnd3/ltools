project (audio)

set(deps
	logging
	testing
	memory
)

bs_generate_package(audio "tier1" "${deps}" "PortAudio")
