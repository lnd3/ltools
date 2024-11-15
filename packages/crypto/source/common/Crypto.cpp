#include "crypto/Crypto.h"

#include "serialization/Base64.h"

#include <array>

namespace l::crypto {

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
		static const std::array<unsigned char, 12> mED25519Prefix = {0x30,0x2a,0x30,0x05,0x06,0x03,0x2b,0x65,0x70,0x03,0x21,0x00};
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
