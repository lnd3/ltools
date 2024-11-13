#include "testing/Test.h"
#include "logging/Log.h"

#include "serialization/Base64.h"

TEST(Base64, Basic) {

	auto message = "test#!%¤14+,<?=";
	auto encoded = l::serialization::base64_encode(message);
	LOG(LogTest) << encoded;
	auto decoded = l::serialization::base64_decode(encoded);
	LOG(LogTest) << decoded;

	TEST_TRUE(decoded == message, "");

	return 0;
}

