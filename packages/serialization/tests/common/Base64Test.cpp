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

TEST(Base64, RandomInput) {
	for (int i = 0; i < 100; i++) {
		std::string message;
		int maxletters = 10 + std::rand() % 50;
		for (int j = 0; j < maxletters; j++) {
			message += std::rand() % 256;
		}
		auto encoded = l::serialization::base64_encode(message);
		LOG(LogTest) << encoded;
		auto decoded = l::serialization::base64_decode(encoded);
		TEST_TRUE(decoded == message, "");
	}

	return 0;
}

