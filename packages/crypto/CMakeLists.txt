project (crypto)

set(deps
	logging
	testing
)

bs_generate_package(crypto "tier1" "${deps}" "ed25519")
