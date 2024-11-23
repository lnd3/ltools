#include "crypto/Crypto.h"

#include "serialization/Base64.h"
#include "serialization/Base16.h"

#include "cryptopp/cryptlib.h"
#include "cryptopp/simple.h"
#include "cryptopp/filters.h"

#include <array>

namespace l::crypto {

	// PKCS#8 format
	std::string ToPublicKeyFormat(std::string_view pkcsFormat) {
		std::stringstream stream;
		stream << "-----BEGIN PUBLIC KEY-----\n";
		stream << pkcsFormat;
		stream << "\n";
		stream << "-----END PUBLIC KEY-----\n";
		return stream.str();
	}

	std::string ToPemKey(std::string_view data, bool b16) {
		static const std::array<unsigned char, 12> mED25519Prefix = { 0x30,0x2a,0x30,0x05,0x06,0x03,0x2b,0x65,0x70,0x03,0x21,0x00 };
		std::string pem(200, '0');
		memcpy(pem.data(), mED25519Prefix.data(), mED25519Prefix.size());
		memcpy(pem.data() + 12, data.data(), data.size());
		auto pemKey = std::string_view(reinterpret_cast<const char*>(pem.data()), 12 + data.size());
		if (b16) {
			return l::serialization::base16_encode(pemKey);
		}
		return l::serialization::base64_encode(pemKey);
	}

	void CryptoXED25519::ResetFromPrivateKey() {
		mVerifier.reset();
		mEd = std::make_unique<CryptoPP::x25519>(mPrivateKey);
		mEd->GeneratePublicKey(CryptoPP::NullRNG(), mPrivateKey, mPublicKey);
		{
			CryptoPP::x25519 xtest(mPublicKey, mPrivateKey);
			ASSERT(xtest.Validate(CryptoPP::NullRNG(), 3));
		}

		mSigner = std::make_unique<CryptoPP::ed25519::Signer>(mPrivateKey);
	}

	void CryptoXED25519::ResetFromPublicKey() {
		mSigner.reset();
		mVerifier = std::make_unique<CryptoPP::ed25519::Verifier>(mPublicKey);
	}

	void CryptoXED25519::CreateNewKeys() {
		mVerifier.reset();
		mEd = std::make_unique<CryptoPP::x25519>();
		mEd->GenerateKeyPair(mRng, mPrivateKey, mPublicKey);
		{
			CryptoPP::x25519 xtest(mPublicKey, mPrivateKey);
			ASSERT(xtest.Validate(CryptoPP::NullRNG(), 3));
		}
		mSigner = std::make_unique<CryptoPP::ed25519::Signer>(mPrivateKey);
		mVerifier = std::make_unique<CryptoPP::ed25519::Verifier>(*mSigner);
	}

	bool CryptoXED25519::LoadPrivateKeyB64(std::string_view privateKeyB64) {
		if (privateKeyB64.empty()) {
			return false;
		}
		auto priKey = l::serialization::base64_decode(privateKeyB64);
		if (priKey.size() != 32) {
			return false;
		}
		memcpy(mPrivateKey, priKey.c_str(), priKey.size());
		ResetFromPrivateKey();
		return true;
	}

	bool CryptoXED25519::LoadPublicKeyB64(std::string_view publicKeyB64) {
		if (publicKeyB64.empty()) {
			return false;
		}
		auto pubKey = l::serialization::base64_decode(publicKeyB64);
		if (pubKey.size() != 32) {
			return false;
		}
		memcpy(mPublicKey, pubKey.c_str(), pubKey.size());
		ResetFromPublicKey();
		return true;
	}

	bool CryptoXED25519::LoadPrivateKeyHex(std::string_view privateKeyHex) {
		if (privateKeyHex.empty() || (privateKeyHex.size() != 64 && privateKeyHex.size() != 128)) {
			return false;
		}
		if (privateKeyHex.size() == 128) {
			l::string::hex_decode(mPrivateKey, privateKeyHex.substr(32));
		}
		else {
			l::string::hex_decode(mPrivateKey, privateKeyHex);
		}
		ResetFromPrivateKey();
		return true;
	}

	bool CryptoXED25519::LoadPublicKeyHex(std::string_view publicKeyHex) {
		if (publicKeyHex.empty() || publicKeyHex.size() != 64) {
			return false;
		}
		l::string::hex_decode(mPublicKey, publicKeyHex);
		ResetFromPublicKey();
		return true;
	}

	void CryptoXED25519::AccumulateMessage(std::string_view message) {
		if (mSigner) {
			if (mPKMessageAccumulator == nullptr) {
				mPKMessageAccumulator = mSigner->NewSignatureAccumulator(CryptoPP::NullRNG());
			}
			mPKMessageAccumulator->Update(reinterpret_cast<const CryptoPP::byte*>(message.data()), message.size());
		}
		else if (mVerifier) {
			if (mSMessageAccumulator == nullptr) {
				mSMessageAccumulator = mVerifier->NewVerificationAccumulator();
			}
			mSMessageAccumulator->Update(reinterpret_cast<const CryptoPP::byte*>(message.data()), message.size());
		}
	}

	CryptoPP::byte* CryptoXED25519::SignMessage() {
		if (mSigner && mPKMessageAccumulator) {
			auto signLength = mSigner->Sign(CryptoPP::NullRNG(), mPKMessageAccumulator, mSignature);
			mPKMessageAccumulator = nullptr;
			if (signLength == 64) {
				return mSignature;
			}
		}
		return nullptr;
	}

	std::string CryptoXED25519::SignMessageB64() {
		auto signature = SignMessage();
		auto signatureData = std::string_view(reinterpret_cast<const char*>(signature), 64);
		return l::serialization::base64_encode(signatureData);
	}

	std::string CryptoXED25519::SignMessagePem() {
		auto signature = SignMessage();
		auto signatureData = std::string_view(reinterpret_cast<const char*>(signature), 64);
		return l::serialization::base64_encode(crypto::ToPemKey(signatureData));
	}

	std::string CryptoXED25519::SignMessageHex() {
		auto signature = SignMessage();
		return l::string::hex_encode(std::string_view(reinterpret_cast<const char*>(signature), 64));
	}

	bool CryptoXED25519::Verify() {
		if (mVerifier && mSMessageAccumulator) {
			auto result = mVerifier->Verify(mSMessageAccumulator);
			mSMessageAccumulator = nullptr;
			return result == 0;
		}
		return false;
	}

	bool CryptoXED25519::VerifyB64(std::string_view signatureB64) {
		if (mVerifier && mSMessageAccumulator) {
			l::serialization::base64_decode(mSignature, signatureB64);
			{
				auto accum = static_cast<CryptoPP::ed25519_MessageAccumulator*>(mSMessageAccumulator);
				memcpy(accum->signature(), mSignature, 64);
			}
			auto result = mVerifier->Verify(mSMessageAccumulator);
			mSMessageAccumulator = nullptr;
			return result;
		}
		return false;
	}

	bool CryptoXED25519::VerifyHex(std::string_view signatureHex) {
		if (mVerifier && mSMessageAccumulator) {
			if (signatureHex.size() != 128) {
				return false;
			}
			l::string::hex_decode(mSignature, signatureHex);
			memcpy(mSMessageAccumulator->signature(), mSignature, 64);
			auto result = mVerifier->Verify(mSMessageAccumulator);
			mSMessageAccumulator = nullptr;
			return result;
		}
		return false;
	}


	CryptoPP::byte* CryptoXED25519::AccessPrivateKey() {
		return mPrivateKey;
	}

	std::string CryptoXED25519::SaveDERPublicKeyHex(bool newFormat) {
		CryptoPP::ByteQueue bt;
		//CryptoPP::x25519 xed(mPublicKey, mPrivateKey);
		mEd->Save(bt, newFormat);
		unsigned char buf[148];
		bt.Get(buf, 48);
		memcpy(buf + 16, mPublicKey, 32);
		return l::string::hex_encode(buf, 48);
	}

	std::string CryptoXED25519::SaveDERPublicKeyB64(bool newFormat) {
		CryptoPP::ByteQueue bt;
		mEd->Save(bt, newFormat);
		unsigned char buf[48];
		bt.Get(buf, 48);
		memcpy(buf + 16, mPublicKey, 32);
		return l::serialization::base64_encode(buf, 48);
	}

	std::string CryptoXED25519::SavePEMPublicKeyB64() {
		return l::serialization::base64_encode(ToPemKey(std::string_view(reinterpret_cast<const char*>(mPublicKey), 32)));
	}

	std::string CryptoXED25519::GetPrivateKeyDER() {
		CryptoPP::ByteQueue bt;
		mEd->DEREncode(bt);
		unsigned char buf[64];
		auto len = bt.Get(buf, 64);

		return l::string::hex_encode(buf, len);
	}

	std::string CryptoXED25519::GetPublicKeyBER() {
		CryptoPP::ByteQueue bt;
		mEd->BEREncode(bt);
		unsigned char buf[64];
		auto len = bt.Get(buf, 64);

		return l::string::hex_encode(buf, len);
	}

	std::string CryptoXED25519::GetPrivateKeyB64() {
		return l::serialization::base64_encode(std::string_view(reinterpret_cast<const char*>(mPrivateKey), 32));
	}

	std::string CryptoXED25519::GetPrivateKeyHex() {
		return l::string::hex_encode(std::string_view(reinterpret_cast<const char*>(mPrivateKey), 32));
	}

	CryptoPP::byte* CryptoXED25519::AccessPublicKey() {
		return mPublicKey;
	}

	std::string CryptoXED25519::GetPublicKeyB64() {
		return l::serialization::base64_encode(std::string_view(reinterpret_cast<const char*>(mPublicKey), 32));
	}

	std::string CryptoXED25519::GetPublicKeyHex() {
		return l::string::hex_encode(std::string_view(reinterpret_cast<const char*>(mPublicKey), 32));
	}

	std::string CryptoXED25519::GetPublicKeyPem(bool b16) {
		return ToPemKey(std::string_view(reinterpret_cast<const char*>(mPublicKey), 32), b16);
	}

	std::string CryptoXED25519::GetPublicKeyPKCS8() {
		return ToPublicKeyFormat(ToPemKey(std::string_view(reinterpret_cast<const char*>(mPublicKey), 32)));
	}












	bool CryptoED25519::Init() {
		memset(mSeed, 0, 32);
		if (ed25519_create_seed(&mSeed[0])) {
			return false;
		}
		bool result = false;
		for (int i = 0; i < 32; i++) {
			if (mSeed[i] != 0) {
				result = true;
				break;
			}
		}
		return result;
	}

	void CryptoED25519::CreateKeys(std::string_view pubKeyBase64, std::string_view priKeyBase64) {
		if (pubKeyBase64.empty() && priKeyBase64.empty()) {
			ed25519_create_keypair(mPubKey, mPriKey, mSeed);
		}
		else {
			if (!pubKeyBase64.empty()) {
				auto pubKey = l::serialization::base64_decode(pubKeyBase64);
				if (pubKey.size() == 32) {
					// raw key
					memcpy(mPubKey, pubKey.c_str(), pubKey.size());
				}
				else if (pubKey.size() == 44) {
					// rsa/pem encoded, first 12 bytes are id bytes
					LOG(LogInfo) << "Loaded public key of type '" << std::string_view(pubKey.c_str(), 12) << "'";
					memcpy(mPubKey, pubKey.c_str() + 12, pubKey.size() - 12);
				}
			}
			if (!priKeyBase64.empty()) {
				auto priKey = l::serialization::base64_decode(priKeyBase64);
				memset(mPriKey, 0, 64);
				memcpy(mPriKey + (64-priKey.size()), priKey.c_str(), priKey.size());
			}
		}
	}

	void CryptoED25519::CreateKeysFromB16(std::string_view pubKeyBase16, std::string_view priKeyBase16) {
		if (pubKeyBase16.empty() && priKeyBase16.empty()) {
			ed25519_create_keypair(mPubKey, mPriKey, mSeed);
		}
		else {
			if (!pubKeyBase16.empty()) {
				auto pubKey = l::serialization::base16_decode(pubKeyBase16);
				if (pubKey.size() == 32) {
					// raw key
					memcpy(mPubKey, pubKey.c_str(), pubKey.size());
				}
				else if (pubKey.size() == 44) {
					// rsa/pem encoded, first 12 bytes are id bytes
					LOG(LogInfo) << "Loaded public key of type '" << std::string_view(pubKey.c_str(), 12) << "'";
					memcpy(mPubKey, pubKey.c_str() + 12, pubKey.size() - 12);
				}
			}
			if (!priKeyBase16.empty()) {
				auto priKey = l::serialization::base16_decode(priKeyBase16);
				memset(mPriKey, 0, 64);
				memcpy(mPriKey + (64 - priKey.size()), priKey.c_str(), priKey.size());
			}
		}
	}
	std::string CryptoED25519::GetSign(std::string_view message) {
		auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
		ed25519_sign(mSign, messagePtr, message.size(), mPubKey, mPriKey);
		auto sign = std::string_view(reinterpret_cast<const char*>(mSign), 64);
		return std::string(sign);
	}

	std::string CryptoED25519::GetSignBase64(std::string_view message) {
		auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
		ed25519_sign(mSign, messagePtr, message.size(), mPubKey, mPriKey);
		auto sign = std::string_view(reinterpret_cast<const char*>(mSign), 64);
		return l::serialization::base64_encode(sign);
	}

	std::string CryptoED25519::GetSignHex(std::string_view message) {
		auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
		ed25519_sign(mSign, messagePtr, message.size(), mPubKey, mPriKey);
		auto sign = std::string_view(reinterpret_cast<const char*>(mSign), 64);
		return l::string::to_hex2(sign);
	}

	std::string CryptoED25519::GetPriKeyBase64() {
		auto priKey = std::string_view(reinterpret_cast<const char*>(mPriKey), 64);
		return l::serialization::base64_encode(priKey);
	}

	std::string CryptoED25519::GetPubKeyBase64() {
		auto pubKey = std::string_view(reinterpret_cast<const char*>(mPubKey), 32);
		return l::serialization::base64_encode(pubKey);
	}

	std::string CryptoED25519::GetPriKeyHex() {
		return l::string::to_hex2(&mPriKey[0], 64);
	}

	std::string CryptoED25519::GetPubKeyHex() {
		return l::string::to_hex2(&mPubKey[0], 32);
	}

	std::string CryptoED25519::GetPubKeyPem() {
		memcpy(mPemKey, "id-Ed25519", 10);
		mPemKey[10] = 1;
		mPemKey[11] = 3;
		mPemKey[12] = 101;
		mPemKey[13] = 112;
		memcpy(mPemKey + 10 + 4, mPubKey, 32);
		auto pemKey = std::string_view(reinterpret_cast<const char*>(mPemKey), 46);
		return l::serialization::base64_encode(pemKey);
	}

	std::string CryptoED25519::GetPubKeyPem2() {
		static const std::array<unsigned char, 12> mED25519Prefix = { 0x30,0x2a,0x30,0x05,0x06,0x03,0x2b,0x65,0x70,0x03,0x21,0x00 };
		memcpy(mPemKey, mED25519Prefix.data(), mED25519Prefix.size());
		memcpy(mPemKey + 12, mPubKey, 32);
		auto pemKey = std::string_view(reinterpret_cast<const char*>(mPemKey), 12 + 32);
		return l::serialization::base64_encode(pemKey);
	}

	/*
	* Binance
	  publicKeyEncoding: {
		type: 'spki',
		format: 'pem'
	  },
	  privateKeyEncoding: {
		type: 'pkcs8',
		format: 'pem'
	
	
	*/
	bool CryptoED25519::Verify(std::string_view signature, std::string_view message, std::string_view publicKey) {
		unsigned char* pubKey = reinterpret_cast<unsigned char*>(const_cast<char*>(publicKey.data()));
		if (publicKey.empty()) {
			pubKey = mPubKey;
		}

		auto signPtr = reinterpret_cast<const unsigned char*>(signature.data());
		auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
		auto result = ed25519_verify(signPtr, messagePtr, message.size(), pubKey);
		return result != 0;
	}

}
