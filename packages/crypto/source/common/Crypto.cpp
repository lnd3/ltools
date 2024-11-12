#include "crypto/Crypto.h"

namespace l::crypto {

	bool Crypto::Init() {
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


}
