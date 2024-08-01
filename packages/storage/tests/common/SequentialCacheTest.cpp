#include "testing/Test.h"

#include "storage/SequentialCache.h"
#include "storage/SequentialCacheGroup.h"
#include "various/serializer/Serializer.h"

#include <memory>
#include <filesystem>

class Data {
public:
	Data() = default;
	~Data() = default;

	friend zpp::serializer::access;
	template <typename Archive, typename Self>
	static void serialize(Archive& archive, Self& self) {
		archive(self.mValue);
	}

	int32_t mValue = 0;
};

TEST(SequentialCache, Setup) {
	l::filecache::FileCacheProvider fileCacheProvider("tests/store", ".test");
	{
		l::filecache::SequentialCacheStore<Data> store(&fileCacheProvider);
		auto block = store.Get("Key", 25, 10);
		block->AllocateBlockData();
		block->Get()->mValue = 1;
		block->PersistData();
	}
	{
		l::filecache::SequentialCacheStore<Data> store(&fileCacheProvider);
		auto block = store.Get("Key", 25, 10);
		block->ProvideData();

		TEST_TRUE(block->HasData(), "");
		TEST_TRUE(block->Get()->mValue == 1, "");
	}
	return 0;
}

TEST(SequentialCache, CacheGroup) {
	l::filecache::SequentialCacheStore<Data> store;

	//auto a = store.GetCache("Key1");
	//l::filecache::CacheGroup<Data, Data, Data> group(a, store.GetCache("Key2"), store.GetCache("Key3"));

	return 0;
}
