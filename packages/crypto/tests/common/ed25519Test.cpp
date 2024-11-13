#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"

using namespace l;

TEST(Crypto, ed2519) {

	std::string message = "test message";
	crypto::CryptoED25519 crypto;
	crypto.CreateKeys();
	auto signature = crypto.Sign(message);
	//LOG(LogTest) << signature;

	TEST_FALSE(crypto.Verify(signature, message + "sd"), "");
	TEST_TRUE(crypto.Verify(signature, message), "");

	return 0;
}
