#include "testing/Test.h"

#include "serialization/SerializationBase.h"

#include <memory>
#include <filesystem>

using namespace l;
using namespace serialization;

class TestData : public SerializationBase {
public:
	TestData(int32_t version, int a, bool useVersion, bool useIdentifier, bool expectVersion, bool expectIdentifier) : SerializationBase(version, 5, useVersion, false, useIdentifier, expectIdentifier, expectVersion), mA(a) {}
	virtual ~TestData() = default;

	friend zpp::serializer::access;
	virtual void Save(SaveArchive& archive) const {
		archive(mA);
	}
	virtual void Load(LoadArchive& archive) {
		archive(mA);
	}

	virtual void Upgrade(int32_t) {

	}

	int mA;
};

class Config3 : public SerializationBase {
public:
	Config3() : SerializationBase(0, 0), mA{} {}
	Config3(int32_t version, int a) : SerializationBase(version, version), mA(a) {}
	virtual ~Config3() = default;

	friend zpp::serializer::access;
	virtual void Save(SaveArchive& archive) const {
		archive(mA);
	}
	virtual void Load(LoadArchive& archive) {
		archive(mA);
	}

	virtual void Upgrade(int32_t) {

	}

	int mA;
};

class Config2 : public SerializationBase {
public:
	static const int32_t LatestVersion = 2;

	Config2() : SerializationBase(0, LatestVersion), mId{}, mNumber{}, mId2{} {}
	Config2(int32_t version, std::string id, uint64_t number, std::string id2) : 
		SerializationBase(version, LatestVersion),
		mId(id), 
		mNumber(number), 
		mId2(id2)
	{}
	virtual ~Config2() = default;

	virtual void Save(SaveArchive& archive) const {
		archive(mId, mData, mNumber, mId2, mConfig);
	}
	friend zpp::serializer::access;
	virtual void Load(LoadArchive& archive) {
		archive(mId, mData, mNumber, mId2, mConfig);
	}

	virtual void Upgrade(int32_t) {
	
	}

	std::string mId;
	std::vector<int> mData;
	uint64_t mNumber;
	std::string mId2;
	Config3 mConfig;
};

TEST(SerializationBase, Basics) {

	std::vector<unsigned char> dataNoVersion = { 0xa, 0x0, 0x0, 0x0 };
	std::vector<unsigned char> dataOnlyVersion = { 0x5, 0x0, 0x0, 0x0, 0xb, 0x0, 0x0, 0x0 };
	std::vector<unsigned char> dataHeaderAndVersion;
	
	{ // load simple data
		TestData storage(3, 10, true, true, false, false);
		storage.LoadArchiveData(dataNoVersion);
		TEST_TRUE(dataNoVersion.empty(), "");
		TEST_TRUE(storage.GetVersion() == 5, "");
		TEST_TRUE(storage.mA == 10, "");

		// serialize and verify header
		storage.GetArchiveData(dataHeaderAndVersion);
		TEST_TRUE(dataHeaderAndVersion.size() == 12, "");
		TEST_TRUE(dataHeaderAndVersion.at(0) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(1) == 0xfa, "");
		TEST_TRUE(dataHeaderAndVersion.at(2) == 0xde, "");
		TEST_TRUE(dataHeaderAndVersion.at(3) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(4) == 5, "");
		TEST_TRUE(dataHeaderAndVersion.at(5) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(6) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(7) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(8) == 10, "");

		TestData storage2(0, 0, true, true, true, true);
		storage2.LoadArchiveData(dataHeaderAndVersion);
		TEST_TRUE(storage2.GetVersion() == 5, "");
		TEST_TRUE(storage2.mA == 10, "");

		// serialize and verify header
		storage2.GetArchiveData(dataHeaderAndVersion);
		TEST_TRUE(dataHeaderAndVersion.size() == 12, "");
		TEST_TRUE(dataHeaderAndVersion.at(0) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(1) == 0xfa, "");
		TEST_TRUE(dataHeaderAndVersion.at(2) == 0xde, "");
		TEST_TRUE(dataHeaderAndVersion.at(3) == 0, "");
		dataHeaderAndVersion.clear();
	}
	{ // have to explicitly set use version to true for versioned only data
		TestData storage(0, 0, true, true, true, false);
		storage.LoadArchiveData(dataOnlyVersion);
		TEST_TRUE(dataOnlyVersion.empty(), "");
		TEST_TRUE(storage.GetVersion() == 5, "");
		TEST_TRUE(storage.mA == 11, "");

		// serialize and verify header
		storage.GetArchiveData(dataHeaderAndVersion);
		TEST_TRUE(dataHeaderAndVersion.size() == 12, "");
		TEST_TRUE(dataHeaderAndVersion.at(0) == 0, "");
		TEST_TRUE(dataHeaderAndVersion.at(1) == 0xfa, "");
		TEST_TRUE(dataHeaderAndVersion.at(2) == 0xde, "");
		TEST_TRUE(dataHeaderAndVersion.at(3) == 0, "");
		dataHeaderAndVersion.clear();
	}
	{


	}
	return 0;
}



class point {
public:
	point() = default;
	point(int x, int y, std::string str) noexcept : m_x(x), m_y(y), m_str(str) {}

	friend zpp::serializer::access;
	template <typename Archive, typename Self>
	static void serialize(Archive& archive, Self& self) {
		archive(self.m_x, self.m_y, self.m_str);
	}

	int get_x() const noexcept {
		return m_x;
	}

	int get_y() const noexcept {
		return m_y;
	}

private:
	int m_x = 0;
	int m_y = 0;
	std::string m_str;
};

TEST(Serialization, ZppSerializer) {
	std::vector<unsigned char> data;
	zpp::serializer::memory_output_archive out(data);

	std::stringstream stream;
	auto path = std::filesystem::path(L"./Debug/store/test_serialization.txt");

	{
		out(point(1337, 1338, "abc"));
		out(point(2, 3, "4"));
		out(point(5, 6, "7"));

		serialization::convert(stream, data);
	}

	data.clear();

	{
		serialization::convert(data, stream);

		point my_point0;
		point my_point1;
		point my_point2;

		zpp::serializer::memory_input_archive in(data);
		in(my_point0, my_point1, my_point2);

		std::cout << my_point2.get_x() << ' ' << my_point2.get_y() << '\n';
	}

	return 0;
}
