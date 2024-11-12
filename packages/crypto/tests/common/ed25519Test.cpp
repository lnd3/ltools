#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"

using namespace l;

TEST(Crypto, ed2519) {

	crypto::Crypto crypto;
	LOG(LogTest) << crypto.Sign("test");

	return 0;
}
