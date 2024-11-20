#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"
#include "cryptopp/xed25519.h"
#include "cryptopp/rdrand.h"
#include "cryptopp/osrng.h"

#include "serialization/Base64.h"

#include <array>

using namespace l;

namespace {
	std::string GetPemKeyFromHex(std::string_view hexstring) {
		static const std::array<unsigned char, 12> mED25519Prefix = { 0x30,0x2a,0x30,0x05,0x06,0x03,0x2b,0x65,0x70,0x03,0x21,0x00 };
		std::string pem(44, '0');
		memcpy(pem.data(), mED25519Prefix.data(), mED25519Prefix.size());
		memcpy(pem.data() + 12, hexstring.data(), 32);
		auto pemKey = std::string_view(reinterpret_cast<const char*>(pem.data()), 12 + 32);
		return l::serialization::base64_encode(pemKey);
	}
}

TEST(Cryptopp, printPemKeys) {
	auto messageHex = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
	auto signatureHex = "dc2a4459e7369633a52b1bf277839a00201009a3efbf3ecb69bea2186c26b58909351fc9ac90b3ecfdfbc7c66431e0303dca179c138ac17ad9bef1177331a704";
	LOG(LogInfo) << "private key pem: " << GetPemKeyFromHex("833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42");

	return 0;
}

TEST(Cryptopp, xed) {
	CryptoPP::x25519 xed;
	CryptoPP::NonblockingRng rand;

	{
		CryptoPP::byte privateKey[32];
		CryptoPP::byte publicKey[32];
		xed.GenerateKeyPair(rand, privateKey, publicKey);
		LOG(LogTest) << "private key: " << l::string::to_hex2(privateKey, 32);
		LOG(LogTest) << "public key: " << l::string::to_hex2(publicKey, 32);
	}

	auto publicKey = l::string::hex_decode("c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac03fa");
	auto message = l::string::hex_decode("c93255d71dcab10e8f379c26200f3c7bd5f09d9bc3068d3ef4edeb4853022b6");
	auto signature = l::string::hex_decode("c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac037a0000000000000000000000000000000000000000000000000000000000000000");
	auto publicKeyData = CryptoPP::BytePtr(publicKey);
	auto messageData = CryptoPP::BytePtr(message);
	auto signatureData = CryptoPP::BytePtr(signature);

	CryptoPP::ed25519::Verifier verifier(publicKeyData);
	auto result = verifier.VerifyMessage(messageData, message.size(), signatureData, signature.size());
	//TEST_TRUE(result, "");

	return 0;
}

TEST(Cryptopp, CryptoXED25519) {

	crypto::CryptoXED25519 crypto;

	auto message = l::string::hex_decode("72");

	{
		crypto.CreateNewKeys();
		crypto.AccumulateMessage(message);
		auto sig2 = crypto.SignMessageHex();

		crypto.AccumulateMessage(message);
		auto result = crypto.VerifyHex(sig2);
		TEST_TRUE(result, "");
	}
	{
		auto privateKeyHex = "4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb";
		auto publicKeyHex = "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c";
		auto signatureHex = "92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00";

		crypto.LoadPrivateKeyHex(privateKeyHex);
		TEST_TRUE(crypto.GetPrivateKeyHex() == privateKeyHex, "");
		TEST_TRUE(crypto.GetPublicKeyHex() == publicKeyHex, "");

		crypto.AccumulateMessage(message);
		auto sig2 = crypto.SignMessageHex();

		TEST_TRUE(sig2 == signatureHex, "");
	}

	return 0;
}
