#pragma once

#include <string>

#include "logging/LoggingAll.h"
#include "ed25519/src/ed25519.h"

namespace l::crypto {
	class CryptoED25519 {
	public:

		CryptoED25519() {
			Init();
		}
		~CryptoED25519() = default;

		bool Init();
		void CreateKeys(std::string_view pubKeyBase64 = "", std::string_view priKeyBase64 = "");
		std::string GetSignBase64(std::string_view message);
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
