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

	template<bool PUBLIC = true, bool X25519 = false, size_t SIZE = (PUBLIC ? 12 : 16)>
	const std::array<unsigned char, SIZE>& Get25519PemPrefix() {

		// secret x25519 pem key in hex
		// 302e020100300506032b65 6e 04220420 
		// e0ce405cf6f4bccac2dafd30e733857a2a7b15b61f7c55bcbcabad9ac0d9d459
		// public x25519 pem key in hex
		// 302a300506032b65 6e 032100 
		// a864d7a35fb86025638dcc703ebb9278e8d5f299cb61cf99d369a3057463dd28

		// secret ed25519 pem key in hex
		// 302e020100300506032b65 70 04220420
		// 92d138faaf4232f5d71a57c4cc96f1871143d8e934ea0cf67cb16a467f1707b2
		// public ed25519 pem key in hex
		// 302a300506032b65 70 032100
		// 4ae579c96ea75e664a060071054e51ce650c968998d5577d9ae489d2e60f7c0f

		if constexpr (PUBLIC) {
			static const std::array<unsigned char, SIZE> prefix = { 
				0x30,0x2a,
				0x30,0x05,
				0x06,0x03,
				0x2b,0x65, 
				(X25519 ? 0x6e : 0x70), 0x03,
				0x21,0x00 };
			return prefix;
		}
		else {
			static const std::array<unsigned char, SIZE> prefix = { 
				0x30,0x2e,
				0x02,0x01,
				0x00,0x30,
				0x05,0x06,
				0x03,0x2b,
				0x65,(X25519 ? 0x6e : 0x70),
				0x04,0x22,
				0x04,0x20 };
			return prefix;
		}
	}

	class CryptoSigner {
	public:
		CryptoSigner() = default;
		virtual ~CryptoSigner() = default;

		virtual std::string_view GetAssociatedKey() = 0;
		virtual void AccumulateMessage(std::string_view message) = 0;
		virtual std::string SignMessageB64() = 0;
		virtual std::string SignMessageHex() = 0;
		virtual bool IsReady() = 0;
	};

	std::string ToPKCSFormat(std::string_view content, bool publicKey = true);
	std::string To25519PemKey(std::string_view rawkey, bool x25519 = true, bool publicKey = true, bool b16 = false);

	class CryptoHMacSha256 : public CryptoSigner {
	public:
		CryptoHMacSha256() = default;
		~CryptoHMacSha256() = default;

		bool IsReady() {
			return !mAssociatedKey.empty() && mSecretLoaded;
		}

		void SetAssociatedKey(std::string_view associatedKey) {
			mAssociatedKey = associatedKey;
		}

		bool LoadSecretKeyAscii(std::string_view privateKeyAscii) {
			if (privateKeyAscii.empty()) {
				mSecretLoaded = false;
				return false;
			}
			auto keyBytes = reinterpret_cast<const CryptoPP::byte*>(privateKeyAscii.data());
			mHmac.SetKey(keyBytes, privateKeyAscii.size());
			mSecretLoaded = true;
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
		CryptoPP::byte mSignature[32];

		std::string mAssociatedKey;
		bool mSecretLoaded = false;
	};

	class CryptoXED25519 : public CryptoSigner {
	public:
		CryptoXED25519() = default;
		~CryptoXED25519() = default;

		void CreateNewKeys();

		bool IsReady() {
			return !mAssociatedKey.empty();
		}

		void SetAssociatedKey(std::string_view associatedKey) {
			mAssociatedKey = associatedKey;
		}

		bool LoadPrivateKeyB64(std::string_view privateKeyB64);
		bool LoadPrivateKeyHex(std::string_view privateKeyHex);
		bool LoadPublicKeyB64(std::string_view publicKeyB64);
		bool LoadPublicKeyHex(std::string_view publicKeyHex);

		CryptoPP::byte* SignMessage();

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

		bool IsReady() {
			return mPriKey[1] != 0 && mPriKey[7] != 0 && mPriKey[21] != 0;
		}

		std::string GetSign(std::string_view message);
		std::string GetSignBase64(std::string_view message);
		std::string GetSignHex(std::string_view message);

		std::string GetPriKeyBase64();
		std::string GetPubKeyBase64();
		std::string GetPriKeyHex();
		std::string GetPubKeyHex();
		std::string GetPubKeyPem();

		bool Verify(std::string_view signature, std::string_view message, std::string_view publicKey = "");
	protected:
		unsigned char mPemKey[64];
		unsigned char mPubKey[32];
		unsigned char mPriKey[64];
		unsigned char mSeed[32];
		unsigned char mSign[64];
	};

}
