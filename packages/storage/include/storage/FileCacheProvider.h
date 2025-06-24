#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <mutex>
#include <functional>
#include <filesystem>

#include <storage/CacheProvider.h>

namespace l::filecache {

	class FileCacheProvider : public l::filecache::ICacheProvider {
	public:
		FileCacheProvider() :
			mLocation("./"),
			mExtension("") {}
		FileCacheProvider(std::string_view location, std::string_view extension) :
			mLocation(location),
			mExtension(extension) {}
		~FileCacheProvider() = default;

		virtual bool PersistData(std::string_view path, const std::vector<unsigned char>& data) override;
		virtual bool ProvideData(std::string_view path, std::vector<unsigned char>& data) override;

		void ScanLocation(
			std::string_view location, 
			std::string_view extension, 
			std::string_view cacheKey,
			std::function<void(int32_t position, int32_t blockwidth)> handler);
	protected:
		std::filesystem::path mLocation;
		std::string mExtension;

		std::mutex mFileMutex;
	};

}