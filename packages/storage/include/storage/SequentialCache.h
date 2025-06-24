#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <map>
#include <mutex>
#include <memory>
#include <optional>

#include "logging/LoggingAll.h"
#include "math/MathConstants.h"
#include "various/serializer/Serializer.h"
#include "concurrency/ObjectLock.h"

#include "LocalStore.h"

#include <storage/FileCacheProvider.h>

namespace l::filecache {

	int32_t GetClampedPosition(int32_t position, int32_t blockWidth);
	int32_t GetClampedPositionOffset(int32_t position, int32_t blockWidth);
	int32_t GetClampedPositionOffsetFromIndex(int32_t index, int32_t blockWidth, int32_t numBlockEntries);

	std::string GetCacheBlockName(
		std::string_view prefix, 
		int32_t blockWidth, 
		int32_t clampedPos);

	template<class T>
	class CacheBlock {
	public:
		CacheBlock() :
			mPath(""),
			mData(nullptr),
			mCacheProvider(nullptr),
			mPersistOnDestruction(false)
		{}

		CacheBlock(std::string_view path, ICacheProvider* provider, bool noProvisioning = false, bool persistOnDestruction = false) :
			mPath(path),
			mData(nullptr),
			mCacheProvider(provider),
			mPersistOnDestruction(persistOnDestruction)
		{
			if (!noProvisioning) {
				ProvideData();
			}
		}
		~CacheBlock() {
			if (mPersistOnDestruction) {
				PersistData();
			}
		}

		friend zpp::serializer::access;
		template <typename Archive, typename Self>
		static void serialize(Archive& archive, Self& self) {
			archive(*self.mData.get());
		}

		bool PersistData() {
			if (!mCacheProvider) {
				return false;
			}

			std::vector<unsigned char> data;
			GetArchiveData(data);

			std::lock_guard lock(mPathMutex);
			return mCacheProvider->PersistData(mPath, data);
		}

		bool ProvideData() {
			if (!mCacheProvider) {
				return false;
			}

			std::vector<unsigned char> data;
			{
				std::lock_guard lock(mPathMutex);
				if (!mCacheProvider->ProvideData(mPath, data)) {
					return false;
				}
				LoadArchiveData(data);
			}
			return true;
		}

		bool GetArchiveData(std::vector<unsigned char>& data) {
			std::lock_guard lock(mDataMutex);
			if (!mData) {
				return false;
			}

			if constexpr (std::is_base_of_v<l::serialization::SerializationBase, T>) {
				auto sb = reinterpret_cast<l::serialization::SerializationBase*>(mData.get());
				if (sb != nullptr) {
					sb->GetArchiveData(data);
					return true;
				}
			}

			zpp::serializer::memory_output_archive out(data);
			out(*this);

			return true;
		}

		bool LoadArchiveData(std::vector<unsigned char>& data) {
			if (data.size() > 0) {
				std::lock_guard lock(mDataMutex);
				if (!mData) {
					mData = std::make_unique<T>();
				}
				if constexpr (std::is_base_of_v<l::serialization::SerializationBase, T>) {
					auto sb = reinterpret_cast<l::serialization::SerializationBase*>(mData.get());
					if (sb != nullptr) {
						sb->LoadArchiveData(data);
						return true;
					}
				}

				zpp::serializer::memory_input_archive in(data);
				in(*this);
				return true;
			}
			return false;
		}

		bool HasData() {
			std::lock_guard<std::mutex> lock(mDataMutex);
			return mData != nullptr;
		}

		void AllocateBlockData(int32_t blockSize = 1) {
			std::lock_guard<std::mutex> lock(mDataMutex);
			if (!mData) {
				mData = std::make_unique<T>(blockSize);
			}
		}

		l::concurrency::ObjectLock<T> Get() {
			mDataMutex.lock();
			return l::concurrency::ObjectLock<T>(mDataMutex, mData.get());
		}

	protected:
		std::mutex mDataMutex;
		std::unique_ptr<T> mData;

		std::string mPath;
		std::mutex mPathMutex;
		ICacheProvider* mCacheProvider;

		bool mPersistOnDestruction;
	};

	template<class T>
	class SequentialCache {
	public:
		SequentialCache(
			std::string_view cacheKey, 
			int32_t cacheBlockWidth, 
			ICacheProvider* cacheProvider
		) :
			mCacheKey(cacheKey),
			mCacheBlockWidth(cacheBlockWidth),
			mCacheProvider(cacheProvider)
		{
			ASSERT(cacheBlockWidth > 0) << "Cache block width cannot be zero";
			//ScanLocation(mCacheKey);
		}
		~SequentialCache() = default;

		bool Has(int32_t position) {
			auto clampedPos = GetClampedPosition(position, mCacheBlockWidth);

			std::lock_guard<std::mutex> lock(mMutexCacheBlockMap);
			auto it = mCacheBlockMap.find(clampedPos);
			if (it != mCacheBlockMap.end()) {
				return true;
			}
			return false;
		}

		CacheBlock<T>* Get(int32_t position, bool noProvisioning = false) {
			auto clampedPos = GetClampedPosition(position, mCacheBlockWidth);

			std::lock_guard<std::mutex> lock(mMutexCacheBlockMap);
			auto it = mCacheBlockMap.find(clampedPos);
			if (it == mCacheBlockMap.end()) {
				auto filename = GetCacheBlockName(mCacheKey, mCacheBlockWidth, clampedPos);
				mCacheBlockMap.emplace(clampedPos, std::make_unique<CacheBlock<T>>(filename, mCacheProvider, noProvisioning));
				it = mCacheBlockMap.find(clampedPos);
			}

			return it->second.get();
		}

		int32_t GetBlockWidth() {
			return mCacheBlockWidth;
		}

	protected:
		std::string mCacheKey;
		int32_t mCacheBlockWidth;

		std::map<int32_t, std::unique_ptr<CacheBlock<T>>> mCacheBlockMap;
		std::mutex mMutexCacheBlockMap;
		ICacheProvider* mCacheProvider;
	};

	template<class T>
	class SequentialCacheStore {
	public:
		SequentialCacheStore(ICacheProvider* cacheProvider = nullptr) :
			mCacheProvider(cacheProvider)
		{}
		~SequentialCacheStore() = default;

		bool Has(std::string_view cacheKey, int32_t position) {
			std::unique_lock<std::mutex> lock(mMutexSequentialCacheMap);
			auto it = mSequentialCacheMap.find(cacheKey.data());
			if (it == mSequentialCacheMap.end()) {
				return false;
			}
			SequentialCache<T>* sequentialCacheMap = it->second.get();
			lock.unlock();

			return sequentialCacheMap->Has(position);
		}

		int32_t GetBlockWidth(std::string_view cacheKey) {
			std::unique_lock<std::mutex> lock(mMutexSequentialCacheMap);
			auto it = mSequentialCacheMap.find(cacheKey.data());
			if (it == mSequentialCacheMap.end()) {
				return 0;
			}
			SequentialCache<T>* sequentialCacheMap = it->second.get();
			lock.unlock();

			return sequentialCacheMap->GetBlockWidth();
		}

		bool ForEach(
			std::string_view cacheKey,
			int32_t beginPosition,
			int32_t endPosition,
			int32_t blockWidth,
			std::function<bool(int32_t start, int32_t size, CacheBlock<T>*)> callback) {

			std::unique_lock<std::mutex> lock(mMutexSequentialCacheMap);
			auto it = mSequentialCacheMap.find(cacheKey.data());
			if (it == mSequentialCacheMap.end()) {
				mSequentialCacheMap.emplace(cacheKey.data(),
					std::make_unique<SequentialCache<T>>(
						cacheKey,
						blockWidth,
						mCacheProvider));
				it = mSequentialCacheMap.find(cacheKey.data());
			}

			SequentialCache<T>* sequentialCacheMap = it->second.get();
			lock.unlock();

			auto cacheBlockWidth = sequentialCacheMap->GetBlockWidth();
			CacheBlock<T>* cacheBlock = nullptr;

			beginPosition = GetClampedPosition(beginPosition, cacheBlockWidth);
			if (beginPosition < endPosition) {
				do {
					cacheBlock = sequentialCacheMap->Get(beginPosition);
					if (cacheBlock != nullptr) {
						if (!callback(beginPosition, cacheBlockWidth, cacheBlock)) {
							break;
						}
					}
					if (static_cast<int64_t>(beginPosition) + cacheBlockWidth >= l::math::constants::INTMAX) {
						break;
					}
					beginPosition += cacheBlockWidth;
				} while (beginPosition <= endPosition);
			}
			else {
				do {
					cacheBlock = sequentialCacheMap->Get(beginPosition);
					if (cacheBlock != nullptr) {
						if (!callback(beginPosition, cacheBlockWidth, cacheBlock)) {
							break;
						}
					}
					if (static_cast<int64_t>(beginPosition) - cacheBlockWidth<= l::math::constants::INTMIN) {
						break;
					}
					beginPosition -= cacheBlockWidth;
				} while (beginPosition >= endPosition);
			}
			return cacheBlock != nullptr;
		}

		bool ForEach2(
			std::string_view cacheKey1,
			std::string_view cacheKey2,
			int32_t beginPosition,
			int32_t endPosition,
			int32_t blockWidth,
			std::function<bool(int32_t, int32_t, CacheBlock<T>*, CacheBlock<T>*)> callback) {

			std::unique_lock<std::mutex> lock(mMutexSequentialCacheMap);
			auto it1 = mSequentialCacheMap.find(cacheKey1.data());
			if (it1 == mSequentialCacheMap.end()) {
				mSequentialCacheMap.emplace(cacheKey1.data(),
					std::make_unique<SequentialCache<T>>(
						cacheKey1,
						blockWidth,
						mCacheProvider));
				it1 = mSequentialCacheMap.find(cacheKey1.data());
			}

			SequentialCache<T>* sequentialCacheMap1 = it1->second.get();
			SequentialCache<T>* sequentialCacheMap2 = nullptr;

			if (!cacheKey2.empty()) {
				auto it2 = mSequentialCacheMap.find(cacheKey2.data());
				if (it2 == mSequentialCacheMap.end()) {
					mSequentialCacheMap.emplace(cacheKey2.data(),
						std::make_unique<SequentialCache<T>>(
							cacheKey2,
							blockWidth,
							mCacheProvider));
					it2 = mSequentialCacheMap.find(cacheKey2.data());
				}
				sequentialCacheMap2 = it2->second.get();
			}

			lock.unlock();

			auto cacheBlockWidth1 = sequentialCacheMap1->GetBlockWidth();
			if (sequentialCacheMap2 != nullptr) {
				auto cacheBlockWidth2 = sequentialCacheMap2->GetBlockWidth();
				ASSERT(cacheBlockWidth1 == cacheBlockWidth2);
			}

			CacheBlock<T>* cacheBlock1 = nullptr;
			CacheBlock<T>* cacheBlock2 = nullptr;
			beginPosition = GetClampedPosition(beginPosition, cacheBlockWidth1);
			if (beginPosition <= endPosition) {
				do {
					cacheBlock1 = sequentialCacheMap1->Get(beginPosition);
					if (cacheBlock1 != nullptr) {
						if (sequentialCacheMap2 != nullptr) {
							cacheBlock2 = sequentialCacheMap2->Get(beginPosition);
						}
						if (!callback(beginPosition, cacheBlockWidth1, cacheBlock1, cacheBlock2)) {
							break;
						}
					}
					if (static_cast<int64_t>(beginPosition) + cacheBlockWidth1 >= l::math::constants::INTMAX) {
						break;
					}
					beginPosition += cacheBlockWidth1;
				} while (beginPosition <= endPosition);
			}
			else {
				do {
					cacheBlock1 = sequentialCacheMap1->Get(beginPosition);
					if (cacheBlock1 != nullptr) {
						if (sequentialCacheMap2 != nullptr) {
							cacheBlock2 = sequentialCacheMap2->Get(beginPosition);
						}
						if (!callback(beginPosition, cacheBlockWidth1, cacheBlock1, cacheBlock2)) {
							break;
						}
					}
					if (static_cast<int64_t>(beginPosition) - cacheBlockWidth1 <= l::math::constants::INTMIN) {
						break;
					}
					beginPosition -= cacheBlockWidth1;
				} while (beginPosition >= endPosition);
			}
			return cacheBlock1 != nullptr && (sequentialCacheMap2 == nullptr || cacheBlock2 != nullptr);
		}

		CacheBlock<T>* Get(std::string_view cacheKey, int32_t position, int32_t blockWidth, bool noProvisioning = false) {
			std::unique_lock<std::mutex> lock(mMutexSequentialCacheMap);
			auto it = mSequentialCacheMap.find(cacheKey.data());
			if (it == mSequentialCacheMap.end()) {
				mSequentialCacheMap.emplace(cacheKey.data(),
					std::make_unique<SequentialCache<T>>(
						cacheKey,
						blockWidth,
						mCacheProvider));
				it = mSequentialCacheMap.find(cacheKey.data());
			}
			SequentialCache<T>* sequentialCacheMap = it->second.get();
			lock.unlock();

			return sequentialCacheMap->Get(position, noProvisioning);
		}

		SequentialCache<T>* GetCache(std::string_view cacheKey) {
			std::unique_lock<std::mutex> lock(mMutexSequentialCacheMap);
			auto it = mSequentialCacheMap.find(cacheKey.data());
			if (it == mSequentialCacheMap.end()) {
				return nullptr;
			}

			SequentialCache<T>* sequentialCacheMap = it->second.get();
			lock.unlock();

			return sequentialCacheMap;
		}

	protected:
		std::map<std::string, std::unique_ptr<SequentialCache<T>>> mSequentialCacheMap;
		std::mutex mMutexSequentialCacheMap;
		ICacheProvider* mCacheProvider;
	};

	template<class T>
	auto CreateSequentialCacheStore(
		ICacheProvider* cacheProvider) {
		return std::make_unique<SequentialCacheStore<T>>(cacheProvider);
	}
}