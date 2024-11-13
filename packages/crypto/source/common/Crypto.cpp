#include "crypto/Crypto.h"

#include "serialization/Base64.h"

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
					memcpy(mPubKey, pubKey.c_str(), pubKey.size());
				}
			}
			if (!priKeyBase64.empty()) {
				auto priKey = l::serialization::base64_decode(priKeyBase64);
				if (priKey.size() == 64) {
					memcpy(mPriKey, priKey.c_str(), priKey.size());
				}
			}
		}
	}

	std::string_view CryptoED25519::Sign(std::string_view message) {
		auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
		ed25519_sign(mSign, messagePtr, message.size(), mPubKey, mPriKey);
		return std::string_view(reinterpret_cast<const char*>(mSign), 64);
	}

	std::string CryptoED25519::GetPriKeyBase64() {
		auto priKey = std::string_view(reinterpret_cast<const char*>(mPriKey), 64);
		return l::serialization::base64_encode(priKey);
	}

	std::string CryptoED25519::GetPubKeyBase64() {
		auto pubKey = std::string_view(reinterpret_cast<const char*>(mPubKey), 32);
		return l::serialization::base64_encode(pubKey);
	}

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
