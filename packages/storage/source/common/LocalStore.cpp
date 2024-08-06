#include "storage/LocalStore.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <atomic>
#include <iostream>
#include <ctime>

namespace l {
namespace storage {

	const int32_t HeaderValidity::kIdentifier = 0x00defa00; // storage base file identifier

	std::atomic_int32_t count_reads_ops = 0;
	std::atomic_int32_t count_write_ops = 0;
	std::atomic_bool abort_all_reads = false;
	std::atomic_bool abort_all_writes = false;

	bool read(const std::filesystem::path& mLocation, char* dst, size_t count, size_t position) {
		if (!std::filesystem::exists(mLocation)) {
			return false;
		}

		std::ifstream ifs(mLocation);
		if (!ifs.good()) {
			return false;
		}
		if (position > 0) {
			ifs.seekg(position);
		}
		ifs.read(dst, static_cast<std::streamsize>(count));
		return true;
	}

	bool write(const std::filesystem::path& mLocation, const char* src, size_t count, size_t position) {
		if (!std::filesystem::exists(mLocation)) {
			if (!std::filesystem::create_directories(mLocation.parent_path())) {
				return false;
			}
		}

		std::ofstream ofs(mLocation);
		if (!ofs.good()) {
			return false;
		}
		if (position > 0) {
			ofs.seekp(position);
		}
		ofs.write(src, static_cast<std::streamsize>(count));
		return true;
	}


	bool read(const std::filesystem::path& mLocation, std::stringstream& data) {
		if (!std::filesystem::exists(mLocation)) {
			return false;
		}

		std::ifstream ifs(mLocation);
		count_reads_ops++;
		std::array<char, 2048> buf;
		for (;;) {
			ifs.read(buf.data(), buf.size());
			size_t count = static_cast<size_t>(ifs.gcount());
			if (count <= 0 || abort_all_reads) {
				break;
			}

			storage::convert(data, buf, count);
		}
		ifs.close();
		count_reads_ops--;

		return true;
	}

	bool write(const std::filesystem::path& mLocation, std::stringstream& data) {
		if (std::filesystem::create_directories(mLocation.parent_path())) {
			return false;
		}

		std::ofstream ofs(mLocation, std::ofstream::out);
		count_write_ops++;
		std::array<char, 2048> buf;
		for (;;) {
			data.read(buf.data(), buf.size());
			size_t count = static_cast<size_t>(data.gcount());
			if (count <= 0 || abort_all_writes) {
				break;
			}
			ofs.write(buf.data(), count);
		}
		ofs.close();
		count_write_ops--;

		return true;
	}

	LocalStorage::LocalStorage(std::filesystem::path&& location) : mLocation(location) {
		CreatePath();
	}

	void LocalStorage::CreatePath() {
		if (!std::filesystem::exists(mLocation)) {
			EXPECT(std::filesystem::create_directories(mLocation)) << "Failed to create directory: " << mLocation;
		}
	}

}
}
