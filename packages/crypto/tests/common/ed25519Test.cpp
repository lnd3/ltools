﻿#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"
#include "serialization/Base64.h"
#include "serialization/Base16.h"

using namespace l;

TEST(Cryptopp, ed25519test) {
	// verified using openssl : 
	// openssl.exe genpkey -algorithm ed25519 -out ed25519.pem
	// openssl.exe pkey -in ed25519.pem -pubout

	// proper ed25519 keys
	auto skpemCorrect = "MC4CAQAwBQYDK2VwBCIEIJLROPqvQjL11xpXxMyW8YcRQ9jpNOoM9nyxakZ/Fwey";
	auto pkpemCorrect = "MCowBQYDK2VwAyEASuV5yW6nXmZKBgBxBU5RzmUMlomY1Vd9muSJ0uYPfA8=";
	auto skRawCorrect = l::serialization::base64_decode(skpemCorrect);
	auto pkRawCorrect = l::serialization::base64_decode(pkpemCorrect);

	auto skpemCorrectHex = l::serialization::base16_encode(l::serialization::base64_decode(skpemCorrect));
	auto pkpemCorrectHex = l::serialization::base16_encode(l::serialization::base64_decode(pkpemCorrect));
	LOG(LogInfo) << "Secret pem key in hex: " << skpemCorrectHex;
	LOG(LogInfo) << "Public pem key in hex: " << pkpemCorrectHex;

	unsigned char seed[32];
	unsigned char sk[64];
	unsigned char pk[32];
	unsigned char sg[64];

	ed25519_create_seed(seed);
	auto seedStr = std::string_view(reinterpret_cast<const char*>(seed), 32);
	auto seedHex = l::serialization::base16_encode(seedStr);

	ed25519_create_keypair(pk, sk, seed);
	auto skStr = std::string_view(reinterpret_cast<const char*>(sk), 64);
	auto pkStr = std::string_view(reinterpret_cast<const char*>(pk), 32);
	auto skHex = l::serialization::base16_encode(skStr);
	auto pkHex = l::serialization::base16_encode(pkStr);

	LOG(LogInfo) << "seed hex: " << seedHex;
	LOG(LogInfo) << "secret key hex: " << skHex;
	LOG(LogInfo) << "public key hex: " << pkHex;

	auto message = std::string_view("testmessage");

	ed25519_sign(sg, reinterpret_cast<const unsigned char*>(message.data()), message.size(), pk, sk);
	auto sgStr = std::string_view(reinterpret_cast<const char*>(sg), 64);
	auto sgHex = l::serialization::base16_encode(sgStr);

	LOG(LogInfo) << "signature of 'testmessage': " << sgHex;

	return 0;
}



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
	LOG(LogTest) << "public key pem2: " << ed25519.GetPubKeyPem();

	auto signature = ed25519.GetSign(message);

	LOG(LogTest) << "signature:" << signature;

	TEST_FALSE(ed25519.Verify(signature, message + "s", pubKey), "");
	TEST_TRUE(ed25519.Verify(signature, message), pubKey);
	
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

	//                                                                    MCowBQYDK2VwAyEAGb9ECWmEzf6FQbrBZ9w7lshQhqowtrbLDFw4rXAxZuE=
	return 0;
}

TEST(Crypto, generate) {
	crypto::CryptoED25519 ed25519;
	ed25519.CreateKeys();
	auto message = "TestMessage";
	auto sign = ed25519.GetSign(message);
	LOG(LogTest) << "public key: " << ed25519.GetPubKeyHex();
	LOG(LogTest) << "private key: " << ed25519.GetPriKeyHex();
	LOG(LogTest) << "message: " << message;
	LOG(LogTest) << "signature: " << l::serialization::base16_encode(sign);
	return 0;
}

TEST(Crypto, validation) {
	crypto::CryptoED25519 ed25519;
	auto secret = l::string::hex_decode("833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42");
	TEST_TRUE(l::string::to_hex2(secret) == "833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42", "");
	auto publicKey = l::string::hex_decode("ec172b93ad5e563bf4932c70e1245034c35467ef2efd4d64ebf819683467e2bf");
	auto message = l::string::hex_decode("ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
	auto signature = l::string::hex_decode("dc2a4459e7369633a52b1bf277839a00201009a3efbf3ecb69bea2186c26b58909351fc9ac90b3ecfdfbc7c66431e0303dca179c138ac17ad9bef1177331a704");
	LOG(LogTest) << "secret hex:" << l::string::to_hex2(secret);
	LOG(LogTest) << "publicKey hex:" << l::string::to_hex2(publicKey);
	LOG(LogTest) << "message hex:" << l::string::to_hex2(message);
	LOG(LogTest) << "signature hex:" << l::string::to_hex2(signature);

	auto secretBase64 = l::serialization::base64_encode(secret);
	auto publicKeyBase64 = l::serialization::base64_encode(publicKey);

	//ed25519.CreateKeys(publicKeyBase64, secretBase64);
	ed25519.CreateKeysFromB16("", "833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42");
	auto sign = ed25519.GetSignHex(message);
	//TEST_TRUE(sign == signature, "");

	return 0;
}

TEST(Crypto, verifification) {
	crypto::CryptoED25519 ed25519;
	auto publicKey = l::string::hex_decode("c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac03fa");
	auto message = l::string::hex_decode("c93255d71dcab10e8f379c26200f3c7bd5f09d9bc3068d3ef4edeb4853022b6");
	auto signature = l::string::hex_decode("c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac037a0000000000000000000000000000000000000000000000000000000000000000");

	//TEST_TRUE(ed25519.Verify(signature, message, publicKey), "");

	return 0;
}
