#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "crypto/Crypto.h"
#include "serialization/Base64.h"

using namespace l;

TEST(Crypto, ed2519) {

	auto pubKey = "VBGfgCzo9ILV1gGq0UuqIRwcbL1RMCSxYPGpdjHxaOk=";
	auto priKey = "cHDSXbj/fl6Stmw+jRQp0OkAWs+WI8fmSKh5r80MVUqSp2Vv4kf0ysR/+1eqzGs906G0zxjTDAWzJMH/L2Eylg==";

	std::string message = "test message";
	crypto::CryptoED25519 ed25519;
	ed25519.CreateKeys(pubKey, priKey);

	LOG(LogTest) << "public key: " << ed25519.GetPubKeyBase64();
	LOG(LogTest) << "private key: " << ed25519.GetPriKeyBase64();

	auto signature = ed25519.Sign(message);
	auto signatureBase64 = serialization::base64_encode(signature);

	LOG(LogTest) << signatureBase64;

	auto signature2 = serialization::base64_decode(signatureBase64);

	TEST_TRUE(signature2 == signature, "");
	TEST_FALSE(ed25519.Verify(signature2, message + "s"), "");
	TEST_TRUE(ed25519.Verify(signature2, message), "");

	return 0;
}
