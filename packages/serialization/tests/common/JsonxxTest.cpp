#include <testing/Test.h>
#include <logging/Log.h>

#include <serialization/JsonParser.h>
#include <serialization/JsonBuilder.h>
#include <serialization/JsonSerializationBase.h>
#include <jsonxx/jsonxx.h>

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
	std::stringstream stream;
	json.SetStream(&stream);
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

	LOG(LogTest) << "\n" << stream.str();

	std::string_view correct = "{\"a\":\"astring\",\"b\":\"bstring\",\"c\":{\"ca\":\"dastring\",\"cb\":2},\"d\":[\"dastring\",2]}";
	TEST_TRUE(correct == stream.str(), "");

	jsonxx::Object parser;
	parser.parse(stream);

	TEST_TRUE(parser.get<jsonxx::String>("a") == "astring", "");
	TEST_TRUE(parser.get<jsonxx::String>("b") == "bstring", "");
	auto& c = parser.get<jsonxx::Object>("c");
	TEST_TRUE(c.get<jsonxx::String>("ca") == "dastring", "");
	TEST_TRUE(c.get<jsonxx::Number>("cb") == 2, "");
	auto& d = parser.get<jsonxx::Array>("d");
	auto& d1 = d.get<jsonxx::String>(0);
	auto& d2 = d.get<jsonxx::Number>(1);
	TEST_TRUE(d1 == "dastring", "");
	TEST_TRUE(d2 == 2, "");

	return 0;
}

class JsonData : public l::serialization::StreamSerializationBase {
public:
	JsonData() = default;
	~JsonData() = default;

	bool LoadArchiveData(std::stringstream& src) override {
		jsonxx::Object parser;
		if (parser.parse(src)) {
			mName = parser.get<jsonxx::String>("name");
			mId = parser.get<jsonxx::Number>("id");
			auto& array = parser.get<jsonxx::Array>("array");
			for (auto e : array.values()) {
				auto value = e->get<jsonxx::Number>();
				mArray.push_back(value);
			}
		}
		return false;
	}

	void GetArchiveData(std::stringstream& dst) override {
		l::serialization::JsonBuilder json;

		json.SetStream(&dst);
		json.Begin("");
		json.AddString("name", mName);
		json.AddNumber("id", mId);
		json.Begin("array", true);
		{
			for (auto& e : mArray) {
				json.AddNumber("", e);
			}
		}
		json.End(true);
		json.End();
	}

	std::string mName;
	float mId = 0.0f;
	std::vector<int32_t> mArray;
};


TEST(JsonSerializer, Basic) {

	JsonData data;

	data.mName = "test";
	data.mId = 3432;
	data.mArray.push_back(89);
	data.mArray.push_back(91);
			
	std::stringstream str;
	data.GetArchiveData(str);

	JsonData data2;
	data2.LoadArchiveData(str);

	TEST_TRUE(data2.mId == 3432, "");
	TEST_TRUE(data2.mName == "test", "");
	TEST_TRUE(data2.mArray.at(0) == 89, "");
	TEST_TRUE(data2.mArray.at(1) == 91, "");

	return 0;
}


class JsonData2 : public l::serialization::JsonSerializationBase {
public:
	JsonData2() = default;
	~JsonData2() = default;

	bool LoadArchiveData(l::serialization::JsonValue& root) override {
		mName = root.get("name").as_string();
		mId = root.get("id").as_float();
		auto array = root.get("array").as_array();
		for (; array.has_next();) {
			auto e = array.next();
			auto value = e.as_int32();
			mArray.push_back(value);
		}
		return false;
	}

	void GetArchiveData(l::serialization::JsonBuilder& json) override {
		json.Begin("");
		json.AddString("name", mName);
		json.AddNumber("id", mId);
		json.Begin("array", true);
		{
			for (auto& e : mArray) {
				json.AddNumber("", e);
			}
		}
		json.End(true);
		json.End();
	}

	std::string mName;
	float mId = 0.0f;
	std::vector<int32_t> mArray;
};


TEST(JsonSerializer, Basic2) {

	JsonData2 data;

	data.mName = "test";
	data.mId = 3432.0f;
	data.mArray.push_back(89);
	data.mArray.push_back(91);

	std::stringstream src;
	l::serialization::JsonBuilder builder;
	builder.SetStream(&src);
	data.GetArchiveData(builder);

	l::serialization::JsonParser<200> parser;

	auto stream = src.str();
	auto [result, error] = parser.LoadJson(stream.c_str(), stream.size());
	TEST_TRUE(result, "Error: " + std::to_string(error));
	if (result) {
		auto root = parser.GetRoot();

		JsonData2 data2;
		data2.LoadArchiveData(root);

		TEST_TRUE(data2.mId == 3432.0f, "");
		TEST_TRUE(data2.mName == "test", "");
		TEST_TRUE(data2.mArray.at(0) == 89, "");
		TEST_TRUE(data2.mArray.at(1) == 91, "");
	}

	return 0;
}