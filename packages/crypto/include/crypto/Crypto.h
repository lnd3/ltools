#pragma once

#include <string>

#include "logging/LoggingAll.h"
#include "ed25519/src/ed25519.h"

namespace l::crypto {
	// example
	//"apiKey": "T59MTDLWlpRW16JVeZ2Nju5A5C98WkMm8CSzWC4oqynUlTm1zXOxyauT8LmwXEv9",
	//"signature" : "5942ad337e6779f2f4c62cd1c26dba71c91514400a24990a3e7f5edec9323f90"
	class Crypto {
	public:
		bool Init();
		std::string Sign(std::string_view message, std::string_view pubKey, std::string_view priKey) {
			if (pubKey.size() != 64 && priKey.size() != 64) {
				return "";
			}

			auto messagePtr = reinterpret_cast<const unsigned char*>(message.data());
			ed25519_sign(mSign, messagePtr, message.size(), reinterpret_cast<const unsigned char*>(pubKey.data()), reinterpret_cast<const unsigned char*>(priKey.data()));
			return std::string(&mSign[0], 64);
		}
	protected:
		unsigned char mSeed[32];
		unsigned char mSign[64];
	};

}
