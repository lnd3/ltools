#pragma once

#include <string>

#include "logging/LoggingAll.h"
#include "serialization/Base16.h"
#include "serialization/Base64.h"

#include "cryptopp/xed25519.h"
#include "cryptopp/rdrand.h"
#include "cryptopp/osrng.h"

#include "ed25519/src/ed25519.h"

namespace l::crypto {

	class CryptoSigner {
	public:
		CryptoSigner() = default;
		virtual ~CryptoSigner() = default;

		virtual std::string_view GetAssociatedKey() = 0;
		virtual void AccumulateMessage(std::string_view message) = 0;
		virtual std::string SignMessageB64() = 0;
		virtual std::string SignMessageHex() = 0;

	};

	std::string ToPublicKeyFormat(std::string_view pkcsFormat);
	std::string ToPemKey(std::string_view hexstring, bool b16 = false);

	class CryptoHMacSha256 : public CryptoSigner {
	public:
		CryptoHMacSha256() = default;
		~CryptoHMacSha256() = default;

		void SetAssociatedKey(std::string_view associatedKey) {
			mAssociatedKey = associatedKey;
		}

		bool LoadSecretKeyAscii(std::string_view privateKeyAscii) {
			auto keyBytes = reinterpret_cast<const CryptoPP::byte*>(privateKeyAscii.data());
			mHmac.SetKey(keyBytes, privateKeyAscii.size());
			return true;
		}

		std::string_view GetAssociatedKey() override {
			return mAssociatedKey;
		}
		virtual void AccumulateMessage(std::string_view message) override {
			auto p = reinterpret_cast<const CryptoPP::byte*>(message.data());
			mHmac.Update(p, message.size());
		}

		virtual std::string SignMessageB64() override {
			SignMessage();
			return l::serialization::base64_encode(mSignature, 32);
		}

		virtual std::string SignMessageHex() override {
			SignMessage();
			return l::serialization::base16_encode(mSignature, 32);
		}

	protected:
		void SignMessage() {
			mHmac.Final(mSignature);
		}

		CryptoPP::HMAC<CryptoPP::SHA256> mHmac;
		CryptoPP::byte mSecret[32];
		CryptoPP::byte mSignature[32];

		std::string mAssociatedKey;
	};

	class CryptoXED25519 : public CryptoSigner {
	public:
		CryptoXED25519() = default;
		~CryptoXED25519() = default;

		void CreateNewKeys();

		void SetAssociatedKey(std::string_view associatedKey) {
			mAssociatedKey = associatedKey;
		}

		bool LoadPrivateKeyB64(std::string_view privateKeyB64);
		bool LoadPrivateKeyHex(std::string_view privateKeyHex);
		bool LoadPublicKeyB64(std::string_view publicKeyB64);
		bool LoadPublicKeyHex(std::string_view publicKeyHex);

		CryptoPP::byte* SignMessage();
		std::string SignMessagePem();

		virtual void AccumulateMessage(std::string_view message) override;
		virtual std::string SignMessageB64() override;
		virtual std::string SignMessageHex() override;
		std::string_view GetAssociatedKey() override {
			return "";
		}

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

		std::string GetPublicKeyPem(bool b16 = false);
		std::string GetPublicKeyPKCS8();

	protected:
		void ResetFromPrivateKey();
		void ResetFromPublicKey();
		bool Verify();

		std::unique_ptr<CryptoPP::x25519> mEd;
		CryptoPP::NonblockingRng mRng;
		std::unique_ptr<CryptoPP::ed25519::Signer> mSigner;
		std::unique_ptr<CryptoPP::ed25519::Verifier> mVerifier;
		CryptoPP::PK_MessageAccumulator* mPKMessageAccumulator = nullptr;
		CryptoPP::ed25519_MessageAccumulator* mSMessageAccumulator = nullptr;

		CryptoPP::byte mPrivateKey[32];
		CryptoPP::byte mPublicKey[32];
		CryptoPP::byte mSignature[64];

		std::string mAssociatedKey;
	};


	class CryptoED25519 {
	public:

		CryptoED25519() {
			Init();
		}
		~CryptoED25519() = default;

		bool Init();
		void CreateKeys(std::string_view pubKeyBase64 = "", std::string_view priKeyBase64 = "");
		void CreateKeysFromB16(std::string_view pubKeyBase16 = "", std::string_view priKeyBase16 = "");

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
