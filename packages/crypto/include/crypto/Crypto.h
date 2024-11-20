#pragma once

#include <string>

#include "logging/LoggingAll.h"
#include "serialization/Base64.h"

#include "cryptopp/xed25519.h"
#include "cryptopp/rdrand.h"
#include "cryptopp/osrng.h"

#include "ed25519/src/ed25519.h"

namespace l::crypto {

	std::string ToPublicKeyFormat(std::string_view pkcsFormat);
	std::string GetPemKeyFromHex(std::string_view hexstring);

	class CryptoXED25519 {
	public:
		CryptoXED25519() = default;
		~CryptoXED25519() = default;

		void CreateNewKeys();

		bool LoadPrivateKeyB64(std::string_view privateKeyB64);
		bool LoadPrivateKeyHex(std::string_view privateKeyHex);
		bool LoadPublicKeyB64(std::string_view publicKeyB64);
		bool LoadPublicKeyHex(std::string_view publicKeyHex);

		void AccumulateMessage(std::string_view message);

		CryptoPP::byte* SignMessage();
		std::string SignMessageB64();
		std::string SignMessageHex();

		bool VerifyB64(std::string_view signatureB64);
		bool VerifyHex(std::string_view signatureHex);

		CryptoPP::byte* AccessPrivateKey();

		std::string SaveDERPublicKeyHex(bool newFormat = true);
		std::string SaveDERPublicKeyB64(bool newFormat = true);
		std::string SavePEMPublicKeyB64();

		std::string GetPrivateKeyDER();
		std::string GetPublicKeyBER();

		std::string GetPrivateKeyB64();
		std::string GetPrivateKeyHex();

		CryptoPP::byte* AccessPublicKey();
		std::string GetPublicKeyB64();
		std::string GetPublicKeyHex();

	protected:
		void ResetFromPrivateKey();
		void ResetFromPublicKey();
		bool Verify();

		CryptoPP::x25519 mEd;
		CryptoPP::NonblockingRng mRng;
		std::unique_ptr<CryptoPP::ed25519::Signer> mSigner;
		std::unique_ptr<CryptoPP::ed25519::Verifier> mVerifier;
		CryptoPP::PK_MessageAccumulator* mMessageAccumulator = nullptr;

		CryptoPP::byte mPrivateKey[32];
		CryptoPP::byte mPublicKey[32];
		CryptoPP::byte mSignature[64];
	};


	class CryptoED25519 {
	public:

		CryptoED25519() {
			Init();
		}
		~CryptoED25519() = default;

		bool Init();
		void CreateKeys(std::string_view pubKeyBase64 = "", std::string_view priKeyBase64 = "");
		std::string GetSign(std::string_view message);
		std::string GetSignBase64(std::string_view message);
		std::string GetSignHex(std::string_view message);

		std::string GetPriKeyBase64();
		std::string GetPubKeyBase64();
		std::string GetPriKeyHex();
		std::string GetPubKeyHex();
		std::string GetPubKeyPem();
		std::string GetPubKeyPem2();


		bool Verify(std::string_view signature, std::string_view message, std::string_view publicKey = "");
	protected:
		unsigned char mPemKey[64];
		unsigned char mPubKey[32];
		unsigned char mPriKey[64];
		unsigned char mSeed[32];
		unsigned char mSign[64];
	};

}
