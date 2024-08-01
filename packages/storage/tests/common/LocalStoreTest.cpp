#include "testing/Test.h"

#include "storage/LocalStore.h"

#include <memory>
#include <filesystem>

using namespace l;
using namespace storage;

class Storage : public SerializationBase {
public:
	Storage(int32_t version, int a, bool useVersion) : SerializationBase(version, 5, useVersion, false), mA(a) {}
	virtual ~Storage() = default;

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
		Storage storage(3, 10, false);
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

		Storage storage2(0, 0, false);
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
		Storage storage(0, 0, true);
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


	return 0;
}


TEST(Storage, Serialization) {

	auto path = std::filesystem::path(L"./Debug");
	LocalStorage store2(path / L"store");

	Config2 conf2(2, "asda", 422, "blabla");
	conf2.mData.push_back(4);
	conf2.mData.push_back(3);
	conf2.mData.push_back(2);
	conf2.mData.push_back(1);

	for (int i = 0; i < 400000; i++) {
		conf2.mData.push_back(i);
	}

	store2.Set("conf", std::make_unique<Config>(1, "GameConfig", "1234"));
	store2.Set("conf2", std::make_unique<Config2>(std::move(conf2)));

	TEST_TRUE(store2.Save<Config>("conf"), "");
	TEST_TRUE(store2.Save<Config2>("conf2"), "");

	auto* p = store2.Get<Config>("conf");
	TEST_TRUE(p->mDefaultUserId == "1234", "");
	TEST_TRUE(p->mName == "GameConfig", "");
	TEST_TRUE(p->GetVersion() == 1, "");

	store2.Erase("conf");
	TEST_TRUE(store2.Load<Config>("conf"), "");
	p = store2.Get<Config>("conf");
	TEST_TRUE(p->mDefaultUserId == "1234", "");
	TEST_TRUE(p->mName == "GameConfig", "");
	TEST_TRUE(p->GetVersion() == 1, "");

	auto* p2 = store2.Get<Config2>("conf2");
	TEST_TRUE(p2->GetVersion() == 2, "");
	TEST_TRUE(p2->mId == "asda", "");
	TEST_TRUE(p2->mNumber == 422, "");
	TEST_TRUE(p2->mData.size() == 400004, "");
	TEST_TRUE(p2->mData.at(1) == 3, "");
	TEST_TRUE(p2->mData.at(3) == 1, "");
	TEST_TRUE(p2->mId2 == "blabla", "");

	store2.Erase("conf2");
	TEST_TRUE(store2.Load<Config2>("conf2"), "");
	p2 = store2.Get<Config2>("conf2");
	TEST_TRUE(p2->GetVersion() == 2, "");
	TEST_TRUE(p2->mId == "asda", "");
	TEST_TRUE(p2->mNumber == 422, "");
	TEST_TRUE(p2->mData.size() == 400004, "");
	TEST_TRUE(p2->mData.at(1) == 3, "");
	TEST_TRUE(p2->mData.at(3) == 1, "");
	TEST_TRUE(p2->mId2 == "blabla", "");

	return 0;
}
