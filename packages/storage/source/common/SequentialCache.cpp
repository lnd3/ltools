#include "storage/SequentialCache.h"
#include "filesystem/File.h"

namespace l::filecache {

	int32_t GetClampedPosition(int32_t position, int32_t blockWidth) {
		return blockWidth * (position / blockWidth);
	}

	std::string GetCacheBlockName(std::string_view prefix, int32_t blockWidth, int32_t clampedPos) {
		std::stringstream name;
		name << prefix.data();
		name << "_" << blockWidth;
		name << "_" << clampedPos;
		return name.str();
	}

	bool FileCacheProvider::PersistData(std::string_view path, const std::vector<unsigned char>& data) {
		if (data.empty()) {
			return false;
		}
		auto file = mLocation / (std::string(path) + mExtension);

		l::filesystem::File f(file);
		f.modeBinary().modeWriteTrunc();
		if (!f.open()) {
			return false;
		}
		f.write(data);
		f.close();
		LOG(LogInfo) << "Saved " << path;

		return true;
	}

	bool FileCacheProvider::ProvideData(std::string_view path, std::vector<unsigned char>& data) {
		auto file = mLocation / (std::string(path) + mExtension);

		if (!std::filesystem::exists(file)) {
			return false;
		}

		l::filesystem::File f(file);
		f.modeBinary().modeRead();
		if (!f.open()) {
			return false;
		}

		if (f.read(data) == 0) {
			return false;
		}
		f.close();

		return true;
	}

	void FileCacheProvider::ScanLocation(
		std::string_view location, 
		std::string_view extension, 
		std::string_view cacheKey, 
		std::function<void(int32_t position, int32_t blockwidth)> handler) {
		std::error_code error;
		auto it = std::filesystem::recursive_directory_iterator(location, std::filesystem::directory_options::skip_permission_denied, error);
		auto end = std::filesystem::end(it);

		while (it != end) {
			auto p = *it;
			it++;

			auto status = p.status(error);
			if (error) {
				error.clear();
				continue;
			}
			if (!p.is_regular_file() || p.path().extension() != extension) {
				continue;
			}

			auto name = p.path().filename().string();
			auto parts = l::string::split(name, "_.");
			if (parts.size() < 4) {
				// files names must contain prefix, block width, id (position) and extension
				// if not, it's invalid, so we skip
				continue;
			}

			auto indexExtension = parts.size() - 1;
			auto indexClampedPosition = parts.size() - 2;
			auto indexBlockWidth = parts.size() - 3;

			auto cacheKeySize = name.size() - parts.at(indexBlockWidth).size() - parts.at(indexClampedPosition).size() - parts.at(indexExtension).size() - 3;
			auto foundCacheKey = name.substr(0, cacheKeySize);

			if (cacheKey != foundCacheKey) {
				continue;
			}

			auto clampedPosition = std::atoi(parts.at(indexClampedPosition).data());
			auto cacheBlockWidth = std::atoi(parts.at(indexBlockWidth).data());

			handler(clampedPosition, cacheBlockWidth);
		}
	}

}
