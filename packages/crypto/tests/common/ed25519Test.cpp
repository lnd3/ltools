#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"
#include "serialization/Base64.h"

using namespace l;

TEST(Crypto, ed2519) {
	auto pubKey2 = "";
	auto pubKey = "";
	auto priKey = "";

	std::string message = "test message";
	crypto::CryptoED25519 ed25519;
	ed25519.CreateKeys(pubKey, priKey);

	LOG(LogTest) << "public key base64: " << ed25519.GetPubKeyBase64();
	LOG(LogTest) << "private key base64: " << ed25519.GetPriKeyBase64();
	LOG(LogTest) << "public key hex: " << ed25519.GetPubKeyHex();
	LOG(LogTest) << "private key hex: " << ed25519.GetPriKeyHex();
	LOG(LogTest) << "public key pem: " << ed25519.GetPubKeyPem();
	LOG(LogTest) << "public key pem2: " << ed25519.GetPubKeyPem2();

	auto signature = ed25519.GetSignBase64(message);

	LOG(LogTest) << "signature:" << signature;

	TEST_FALSE(ed25519.Verify(signature, message + "s"), "");
	TEST_TRUE(ed25519.Verify(signature, message), "");
	
	// Signature
	// r5YGa0VjkFxOgoY1dMyfH6Jf3j0fIzx5oY/V10Xc4b4Mu6tXrF7RZgQXWLCAfonrJdtDKL99wNuB0RGaJSVyAw==
	LOG(LogTest) << "signature key hex: " << l::string::to_hex2(l::serialization::base64_decode(signature));

	// Examples private keys
	// MC4CAQAwBQYDK2VwBCIEINTuctv5E1hK1bbY8fdp+K06/nwoy/HU++CXqI9EdVhC
	LOG(LogTest) << "private key hex: " << l::string::to_hex2(l::serialization::base64_decode(ed25519.GetPriKeyBase64()));
	LOG(LogTest) << l::string::to_hex2(l::serialization::base64_decode("MC4CAQAwBQYDK2VwBCIEINTuctv5E1hK1bbY8fdp+K06/nwoy/HU++CXqI9EdVhC"));

	// Examples public key with prefix separately printed
	LOG(LogTest) << l::string::to_hex2(l::serialization::base64_decode("MCowBQYDK2VwAyEA"));

	LOG(LogTest) << l::string::to_hex2(l::serialization::base64_decode("MCowBQYDK2VwAyEAVBGfgCzo9ILV1gGq0UuqIRwcbL1RMCSxYPGpdjHxaOk="));

	//LOG(LogTest) << l::string::to_hex2(l::serialization::base64_decode("MCowBQYDK2VwAyEAhh0h5M77+TuNChqNfxFiOqAT5fy6UbHsO6M4pDGmEuE="));
	//LOG(LogTest) << l::string::to_hex2(l::serialization::base64_decode("MCowBQYDK2VwAyEAgmDRTtj2FA+wzJUIlAL9ly1eovjLBu7uXUFR+jFULmg="));
	//LOG(LogTest) << l::string::to_hex2(l::serialization::base64_decode("MCowBQYDK2VwAyEAGb9ECWmEzf6FQbrBZ9w7lshQhqowtrbLDFw4rXAxZuE="));
	return 0;
}

TEST(Crypto, generate) {
	crypto::CryptoED25519 ed25519;
	ed25519.CreateKeys();
	LOG(LogTest) << "public key: " << ed25519.GetPubKeyBase64();
	LOG(LogTest) << "private key: " << ed25519.GetPriKeyBase64();
	return 0;
}

