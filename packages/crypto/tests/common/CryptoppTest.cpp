#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"
#include "cryptopp/xed25519.h"
#include "cryptopp/rdrand.h"
#include "cryptopp/osrng.h"

#include "serialization/Base16.h"
#include "serialization/Base64.h"

#include <array>

using namespace l;

TEST(CryptoPP, hmacsha256) {

	CryptoPP::HMAC<CryptoPP::SHA256> mHmac;
	CryptoPP::byte mSecret[32];
	CryptoPP::byte mSignature[32];
	CryptoPP::SHA256 sha256;


	std::string_view privateKeyAscii = "Ab0z9aZvAb0z9aZvAb0z9aZvAb0z9aZvAb0z9aZvAb0z9aZvAb0z9aZvAb0z9aZv";
	std::string_view message = "test";

	auto keyBytes = reinterpret_cast<const CryptoPP::byte*>(privateKeyAscii.data());
	sha256.CalculateDigest(mSecret, keyBytes, privateKeyAscii.size());

	mHmac.SetKey(mSecret, 32);
	const CryptoPP::byte* p = reinterpret_cast<const CryptoPP::byte*>(message.data());
	mHmac.Update(p, message.size());
	mHmac.Final(mSignature);

	//l::serialization::base64_encode(mSignature, 32);

	TEST_TRUE(mHmac.VerifyDigest(mSignature, p, message.size()), "");

	return 0;
}

#ifdef CRYPTOPP_TEST_1
TEST(Cryptopp, printPemKeys) {
	auto messageHex = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
	auto signatureHex = "dc2a4459e7369633a52b1bf277839a00201009a3efbf3ecb69bea2186c26b58909351fc9ac90b3ecfdfbc7c66431e0303dca179c138ac17ad9bef1177331a704";
	LOG(LogInfo) << "private key pem: " << crypto::ToPemKey("833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42");

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

	return 0;
}

bool TestPublicKey(std::string_view privateKeyB16, std::string_view publicKeyB16) {
	TEST_EQ(privateKeyB16.size(), 64, "");
	TEST_EQ(publicKeyB16.size(), 64, "");
	auto privateKey = l::serialization::base16_decode(privateKeyB16);
	auto privateKeyData = CryptoPP::BytePtr(privateKey);
	CryptoPP::x25519 xed(privateKeyData);
	TEST_TRUE(xed.Validate(CryptoPP::NullRNG(), 3), "");
	CryptoPP::byte computedPublicKeyData[32];
	xed.GeneratePublicKey(CryptoPP::NullRNG(), privateKeyData, computedPublicKeyData);
	TEST_TRUE(xed.Validate(CryptoPP::NullRNG(), 3), "");

	auto computedPublicKey = l::serialization::base16_encode(computedPublicKeyData, 32);
	auto publicKeyB16Lowercase = l::serialization::base16_encode(l::serialization::base16_decode(publicKeyB16));
	TEST_TRUE(computedPublicKey == publicKeyB16Lowercase, "");
	return 0;
}

bool TestGeneratedKey() {
	CryptoPP::byte computedPrivateKeyData[32];
	CryptoPP::byte computedPublicKeyData[32];
	{
		CryptoPP::NonblockingRng rand;
		CryptoPP::x25519 xed;
		xed.GenerateKeyPair(rand, &computedPrivateKeyData[0], &computedPublicKeyData[0]);
	}
	CryptoPP::x25519 xed(&computedPrivateKeyData[0]);
	TEST_TRUE(xed.Validate(CryptoPP::NullRNG(), 3), "");
	auto skStr = std::string_view(reinterpret_cast<const char*>(computedPrivateKeyData), 32);
	auto pkStr = std::string_view(reinterpret_cast<const char*>(computedPublicKeyData), 32);
	auto skB16 = l::serialization::base16_encode(skStr);
	auto pkB16 = l::serialization::base16_encode(pkStr);
	LOG(LogInfo) << "generated private:" << skB16;
	LOG(LogInfo) << "generated public:" << pkB16;
	TEST_FALSE(TestPublicKey(skB16, pkB16), "");

	return 0;
}

bool TestSignature(std::string_view privateKeyB16, std::string_view message, std::string_view signatureB16) {
	TEST_EQ(privateKeyB16.size(), 64, "");
	TEST_EQ(signatureB16.size(), 128, "");
	auto privateKey = l::serialization::base16_decode(privateKeyB16);

	auto privateKeyData = CryptoPP::BytePtr(privateKey);
	auto messageData = reinterpret_cast<const unsigned char*>(message.data());

	CryptoPP::x25519 xed(privateKeyData);
	TEST_TRUE(xed.Validate(CryptoPP::NullRNG(), 3), "");

	CryptoPP::byte computedPublicKeyData[32];
	xed.GeneratePublicKey(CryptoPP::NullRNG(), privateKeyData, computedPublicKeyData);
	TEST_TRUE(xed.Validate(CryptoPP::NullRNG(), 3), "");
	CryptoPP::ed25519::Signer signer(computedPublicKeyData, privateKeyData);
	CryptoPP::byte computedSignatureData[64];

	auto messageAccumulator = signer.NewSignatureAccumulator(CryptoPP::NullRNG());
	messageAccumulator->Update(messageData, message.size());
	auto len = signer.Sign(CryptoPP::NullRNG(), messageAccumulator, computedSignatureData);
	auto computedSignature = l::serialization::base16_encode(computedSignatureData, 64);
	auto signatureB16Lowercase = l::serialization::base16_encode(l::serialization::base16_decode(signatureB16));
	TEST_TRUE(computedSignature == signatureB16Lowercase, "");
	return 0;
}

bool TestVerifier(std::string_view publicKeyB16, std::string_view message, std::string_view signatureB16) {
	TEST_EQ(publicKeyB16.size(), 64, "");
	TEST_EQ(signatureB16.size(), 128, "");
	auto publicKey = l::serialization::base16_decode(publicKeyB16);
	auto signature = l::serialization::base16_decode(signatureB16);

	auto publicKeyData = CryptoPP::BytePtr(publicKey);
	auto messageData = reinterpret_cast<const unsigned char*>(message.data());
	auto signatureData = CryptoPP::BytePtr(signature);

	CryptoPP::ed25519::Verifier verifier;
	auto result = verifier.VerifyMessage(messageData, message.size(), signatureData, 64);
	//TEST_TRUE(result, "");
	return 0;
}

TEST(Cryptopp, printgenerated) {
	CryptoPP::byte sk[32];
	CryptoPP::byte pk[32];
	CryptoPP::byte sign[64];
	CryptoPP::NonblockingRng rand;
	CryptoPP::x25519 xed;
	xed.GenerateKeyPair(rand, sk, pk);
	CryptoPP::x25519 xed2(sk);
	CryptoPP::ed25519::Signer signer(sk);
	CryptoPP::ed25519::Verifier verifier(pk);

	auto skB16 = l::serialization::base16_encode(sk, 32);
	auto pkB16 = l::serialization::base16_encode(pk, 32);
	auto message = std::string_view("TestMessage");
	auto len = signer.SignMessage(CryptoPP::NullRNG(), reinterpret_cast<const unsigned char*>(message.data()), message.size(), sign);

	LOG(LogTest) << "private key: " << l::serialization::base16_encode(sk, 32);
	LOG(LogTest) << "public key: " << l::serialization::base16_encode(pk, 32);
	LOG(LogTest) << "message: " << message;
	LOG(LogTest) << "signature: " << l::serialization::base16_encode(sign, 64);
	return 0;
}

TEST(Cryptopp, generated) {
	TEST_TRUE(!TestGeneratedKey(), "");
	return 0;
}

TEST(Cryptopp, test2) {
	auto privateKeyB16 = "A00FADD6D29BE764B851F64F7620E80B700DF65914BED31E486362281BB5D061";
	auto publicKeyB16 = "C5F9F54D52D5A2FB6AE692B1CCE695017C2EAD755B213D46AC6912F7B979C6CD";
	auto messageB16 = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
	auto signatureB16 = "e30fc92c7548d99838f520eda491e5311ed9ce9fa868e5743191abe8d7f1a45e470f10cc9d23ddc3f5c906851c8b3d974c03006b9afc5bd6263d0fd72dcf5b09";
	LOG(LogInfo) << "private:" << privateKeyB16;
	LOG(LogInfo) << "public:" << publicKeyB16;
	TEST_TRUE(!TestPublicKey(privateKeyB16, publicKeyB16), "");
	TEST_TRUE(!TestSignature(privateKeyB16, messageB16, signatureB16), "");
	TEST_TRUE(!TestVerifier(publicKeyB16, messageB16, signatureB16), "");

	return 0;
}

TEST(Cryptopp, test3) {
	auto privateKeyB16 = "566D593740C7A29D07464412B1FC1E01BC09CFE8D0CC0B5B434DB914916C9E57";
	auto publicKeyB16 = "EE8D0405408B1036B046F63923421C87AD9046CFB7FB23ED66A7DB0F6F7EDE90";
	auto message = "TestMessage";
	auto signatureB16 = "743D4194555C5F578F20D859A98DB1F93EB10297609EF3E2A459EE05513CA0D3DBEA5BFDECF17A3A3C9272C24A543882FBF6B717A4E35920CF71C64908C44D0F";
	LOG(LogInfo) << "private:" << privateKeyB16;
	LOG(LogInfo) << "public:" << publicKeyB16;
	TEST_TRUE(!TestPublicKey(privateKeyB16, publicKeyB16), "");
	TEST_TRUE(!TestSignature(privateKeyB16, message, signatureB16), "");
	TEST_TRUE(!TestVerifier(publicKeyB16, message, signatureB16), "");

	return 0;
}

TEST(Cryptopp, PrintPKCS8) {
	auto pk = "9b1f5eeded043385e4f7bc623c5975b90bc8bb3b";

	crypto::CryptoXED25519 crypto;
	crypto.LoadPublicKeyHex(pk);

	LOG(LogTest) << "pk: \n" << pk;
	LOG(LogTest) << "Pem public key: \n" << crypto.GetPublicKeyPem(true);
	LOG(LogTest) << "PKCS8 public key: \n" << crypto.GetPublicKeyPKCS8();
	return 0;
}

TEST(Cryptopp, CryptoXED25519) {

	crypto::CryptoXED25519 crypto;

	auto message = "TestMessage";

	{
		crypto.CreateNewKeys();
		crypto.AccumulateMessage(message);
		auto pk = crypto.GetPublicKeyHex();
		auto sig2 = crypto.SignMessageHex();

		{
			crypto::CryptoXED25519 cryptoV;
			cryptoV.LoadPublicKeyHex(pk);
			cryptoV.AccumulateMessage(message);
			auto result = cryptoV.VerifyHex(sig2);
			TEST_TRUE(result, "");
		}
	}
	{
		auto privateKeyHex = "50da23e671fe838e8b9edb32f822042117c61265a8dffaf02166beda0ecebc5b";
		auto publicKeyHex = "526ef3da9d2b0028bb1a748be7a3f596f58c5f5332600ae155a5c73a1514563a";

		crypto.LoadPrivateKeyHex(privateKeyHex);
		TEST_TRUE(crypto.GetPrivateKeyHex() == privateKeyHex, "");
		TEST_TRUE(crypto.GetPublicKeyHex() == publicKeyHex, "");
		LOG(LogInfo) << "Private key        : " << crypto.GetPrivateKeyHex();
		LOG(LogTest) << "DER0 public key hex: " << crypto.SaveDERPublicKeyHex(false);
		LOG(LogTest) << "DER1 public key hex: " << crypto.SaveDERPublicKeyHex();
		LOG(LogTest) << "DER0 public key b64: " << crypto.SaveDERPublicKeyB64(false);
		LOG(LogTest) << "DER1 public key b64: \n" << crypto::ToPublicKeyFormat(crypto.SaveDERPublicKeyB64(1));
		LOG(LogTest) << "PEM public key: \n" << crypto.GetPublicKeyPem(true);
		LOG(LogTest) << "PKCS8 public key: \n" << crypto.GetPublicKeyPKCS8();
		crypto.AccumulateMessage(message);
		auto signature = crypto.SignMessageB64();
		LOG(LogTest) << "Sign: \n" << signature;

		{
			crypto::CryptoXED25519 cryptoV;
			cryptoV.LoadPublicKeyHex(publicKeyHex);
			cryptoV.AccumulateMessage(message);
			TEST_TRUE(cryptoV.VerifyB64(signature), "");
		}


		//TEST_TRUE(sig2 == signatureHex, "");
	}
	return 0;
}
#endif
