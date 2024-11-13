project (crypto)

set(deps
	logging
	testing

	serialization
)

bs_generate_package(crypto "tier1" "${deps}" "ed25519")
