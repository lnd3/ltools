#include "testing/Test.h"
#include "logging/Log.h"

#include "serialization/JsonBuilder.h"

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

TEST(Jsonxx, MultiJsonObjects) {

	std::stringstream stream;

	char buf[200];

	std::string s1("{\"e\":\"kline\",\"E\":1731096660008}{\"b\":\"data\",\"c\":1731123}");
	memcpy(buf, s1.c_str(), s1.size());
	stream << std::string_view(buf, s1.size());

	{
		jsonxx::Object o;
		TEST_TRUE(o.parse(stream), "");

		TEST_TRUE("kline" == o.get<jsonxx::String>("e"), "");
	}

	{
		jsonxx::Object o;
		TEST_TRUE(o.parse(stream), "");

		TEST_TRUE("data" == o.get<jsonxx::String>("b"), "");
	}

	std::string s2("{\"r\":\"t\",\"s\":4}");
	memcpy(buf, s2.c_str(), s2.size());
	stream << std::string_view(buf, s2.size());

	{
		jsonxx::Object o;
		TEST_TRUE(o.parse(stream), "");

		TEST_TRUE("t" == o.get<jsonxx::String>("r"), "");
	}

	return 0;
}

TEST(JsonBuilder, Basic) {

	l::serialization::JsonBuilder json;

	json.Begin("");
	{
		json.AddString("a", "astring");
		json.AddString("b", "bstring");
		json.Begin("c");
		{
			json.AddString("ca", "dastring");
			json.AddNumber("cb", 2);
		}
		json.End();

		json.Begin("d", true);
		{
			json.AddString("", "dastring");
			json.AddNumber("", 2);
		}
		json.End(true);
	}
	json.End();
	LOG(LogTest) << "\n" << json.GetStream().str();

	std::string_view correct = "{\"a\":\"astring\",\"b\":\"bstring\",\"c\":{\"ca\":\"dastring\",\"cb\":2},\"d\":[\"dastring\",2]}";
	TEST_TRUE(correct == json.GetStream().str(), "");
	return 0;
}

