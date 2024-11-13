#include "crypto/Crypto.h"

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

	void CryptoED25519::CreateKeys() {
		ed25519_create_keypair(mPubKey, mPriKey, mSeed);
	}

	std::string_view CryptoED25519::Sign(std::string_view message) {
		auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
		ed25519_sign(mSign, messagePtr, message.size(), mPubKey, mPriKey);
		return std::string_view(reinterpret_cast<const char*>(mSign), 64);
	}

	std::string_view CryptoED25519::GetPublicKey() {
		return std::string_view(reinterpret_cast<const char*>(mPubKey), 32);
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
