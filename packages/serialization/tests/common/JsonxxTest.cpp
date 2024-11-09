#include "testing/Test.h"
#include "logging/Log.h"

#include "jsonxx/jsonxx.h"

#include <cstring>
#include <string>
#include <string_view>
#include <memory>
#include <iostream>

TEST(Jsonxx, Parse) {

	std::stringstream stream;

	char buf[200];

	std::string s1("{\"e\":\"kline\",\"E\":1731096660008,\"s\":\"OPUSDT\",\"k\":{\"t\":1731096600000,\"T\":1731096659999,\"s\":\"OPUSDT\",\"i");
	memcpy(buf, s1.c_str(), s1.size());
	stream << std::string_view(buf, s1.size());

	jsonxx::Object o;
	TEST_FALSE(o.parse(stream), "");
	stream.clear();
	std::string s2("\":\"2\"}}");
	memcpy(buf, s2.c_str(), s1.size());
	stream << std::string_view(buf, s1.size());
	TEST_FALSE(o.parse(stream), "");
	stream.clear();

	stream.seekg(0);
	stream.seekp(0, std::ios_base::end);
	stream << std::string_view(buf, s1.size());
	TEST_TRUE(o.parse(stream), "");


	return 0;
}

