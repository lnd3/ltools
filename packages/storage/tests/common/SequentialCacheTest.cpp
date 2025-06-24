#include "testing/Test.h"

#include "storage/SequentialCache.h"
#include "storage/SequentialCacheGroup.h"
#include "various/serializer/Serializer.h"

#include <memory>
#include <filesystem>


class CacheBlock {
public:
	CacheBlock() = default;
	~CacheBlock() = default;

};

class Data {
public:
	Data() = default;
	Data(int32_t) {};
	~Data() = default;

	friend zpp::serializer::access;
	template <typename Archive, typename Self>
	static void serialize(Archive& archive, Self& self) {
		archive(self.mValue);
	}

	int32_t mValue = 0;
};

TEST(SequentialCacheStore, Setup) {
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

TEST(SequentialCacheStore, ForEach) {
	{
		for (int32_t blockSize = 2; blockSize < 6; blockSize++) {
			l::filecache::SequentialCacheStore<Data> store;
			for (int32_t start = 0; start < blockSize * 3; start++) {
				for (int32_t length = blockSize / 2; length < 3 * blockSize; length++) {
					int32_t expectedBlockCount = 1 + (start + length) / blockSize - start / blockSize;
					int32_t blockCount = 0;
					store.ForEach("Cache", start, start + length, blockSize, [&](int32_t s, int32_t bs, l::filecache::CacheBlock<Data>* block) {
						TEST_TRUE_NO_RET(block != nullptr, "");
						TEST_TRUE_NO_RET(!block->HasData(), "");
						TEST_TRUE_NO_RET(bs == blockSize, "");
						TEST_TRUE_NO_RET(s == (blockSize * (start / blockSize) + blockSize * blockCount), "");
						blockCount++;
						return true;
						});
					TEST_TRUE(blockCount == expectedBlockCount, "");
				}
			}
		}
	}
	{
		for (int32_t blockSize = 2; blockSize < 6; blockSize++) {
			l::filecache::SequentialCacheStore<Data> store;
			for (int32_t start = 0; start < blockSize * 3; start++) {
				for (int32_t length = blockSize / 2; length < 3 * blockSize; length++) {
					int32_t expectedBlockCount = 1 + (start + length) / blockSize - start / blockSize;
					int32_t blockCount = 0;
					store.ForEach2("Cache", "key", start, start + length, blockSize, [&](int32_t s, int32_t bs, l::filecache::CacheBlock<Data>* block1, l::filecache::CacheBlock<Data>* block2) {
						TEST_TRUE_NO_RET(block1 != nullptr, "");
						TEST_TRUE_NO_RET(block2 != nullptr, "");
						TEST_TRUE_NO_RET(!block1->HasData(), "");
						TEST_TRUE_NO_RET(!block2->HasData(), "");
						TEST_TRUE_NO_RET(bs == blockSize, "");
						TEST_TRUE_NO_RET(s == (blockSize * (start / blockSize) + blockSize * blockCount), "");
						blockCount++;
						return true;
						});
					TEST_TRUE(blockCount == expectedBlockCount, "");
				}
			}
		}
	}
	return 0;
}

TEST(SequentialCacheStore, CacheGroup) {
	l::filecache::SequentialCacheStore<Data> store;

	//auto a = store.GetCache("Key1");
	//l::filecache::CacheGroup<Data, Data, Data> group(a, store.GetCache("Key2"), store.GetCache("Key3"));

	return 0;
}
