#include "filesystem/File.h"

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <atomic>
#include <iostream>
#include <ctime>
#include <chrono>

namespace l {
namespace filesystem {

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

			convert(data, buf, count);
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

	std::time_t getTime(std::filesystem::file_time_type tp) {
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - std::filesystem::file_time_type::clock::now()
			+ std::chrono::system_clock::now());
		return std::chrono::system_clock::to_time_t(sctp);
	}

	std::string toString(std::time_t t, std::string format) {
		struct std::tm tminfo {};
#ifdef WIN32
		localtime_s(&tminfo, &t);
#else
		localtime_r(&t, &tminfo);
#endif

		std::ostringstream out;
		out << std::put_time(&tminfo, format.c_str());
		return out.str();
	}

	File::File(std::filesystem::path filePath) : mFilePath(filePath), mFileMode(0), mFileStream(std::nullopt) {}

	File::~File() {
		close();
	}

	File& File::modeBinary() {
		mFileMode |= std::ios::binary;
		return *this;
	}

	File& File::modeRead() {
		mFileMode |= std::ios::in;
		return *this;
	}

	File& File::modeReadPreload() {
		mFileMode |= std::ios::ate | std::ios::in;
		return *this;
	}

	File& File::modeWrite() {
		if (mFilePath.has_parent_path() && !std::filesystem::exists(mFilePath.parent_path())) {
			std::filesystem::create_directories(mFilePath.parent_path());
		}
		mFileMode |= std::ios::out;
		return *this;
	}

	File& File::modeWriteTrunc() {
		if (mFilePath.has_parent_path() && !std::filesystem::exists(mFilePath.parent_path())) {
			std::filesystem::create_directories(mFilePath.parent_path());
		}
		mFileMode |= std::ios::out | std::ios::trunc;
		return *this;
	}

	bool File::open() {
		if (mFileStream) {
			return false;
		}

		mFileStream = std::fstream(mFilePath, static_cast<std::ios_base::openmode>(mFileMode));

		if (!mFileStream->good()) {
			close();
			return false;
		}
		setReadPosition(0);
		return true;
	}

	void File::close() {
		if (mFileStream) {
			mFileStream->flush();
			mFileStream->close();
			mFileStream = std::nullopt;
		}
	}

	void File::flush() {
		if (mFileStream) {
			mFileStream->flush();
		}
	}

	std::time_t File::lastWriteTime() {
		auto lastWrite = std::filesystem::last_write_time(mFilePath);
		return getTime(lastWrite);
	}

	std::size_t File::fileSize() {
		auto size = std::filesystem::file_size(mFilePath);
		return static_cast<size_t>(size);
	}

	std::string File::lastWriteAsString() {
		auto lastWrite = std::filesystem::last_write_time(mFilePath);
		auto time = getTime(lastWrite);
		return toString(time);
	}

	void File::setReadPosition(size_t position) {
		if (mFileStream) {
			mFileStream->seekg(position);
		}
	}

	void File::setWritePosition(size_t position) {
		if (mFileStream) {
			mFileStream->seekp(position);
		}
	}

	size_t File::getReadPosition() {
		if (mFileStream) {
			auto c = static_cast<int64_t>(mFileStream->tellg());
			return c < 0 ? 0 : static_cast<size_t>(c);
		}
		ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
		return 0;
	}

	size_t File::getWritePosition() {
		if (mFileStream) {
			auto c = static_cast<int64_t>(mFileStream->tellp());
			return c < 0 ? 0 : static_cast<size_t>(c);
		}
		ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
		return 0;
	}

	size_t File::getLastReadCount() {
		auto g = static_cast<int64_t>(mFileStream->gcount());
		return g < 0 ? 0 : static_cast<size_t>(g);
	}

	size_t File::read(char* dst, size_t count) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		mFileStream->read(dst, count);
		auto c = getLastReadCount();
		ASSERT(count == c) << "Failed to read " << (count - c) << " bytes (" << c << "/" << count << ")";
		return c;
	}

	size_t File::write(const char* src, size_t count) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		mFileStream->write(src, count);
		return count;
	}

	size_t File::read(std::vector<char>& dst) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		auto size = fileSize();
		dst.resize(size);
		mFileStream->read(dst.data(), size);
		auto c = getLastReadCount();
		ASSERT(size == c) << "Failed to read " << (size - c) << " bytes (" << c << "/" << size << ")";
		return c;
	}

	size_t File::write(const std::vector<char>& src) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		mFileStream->write(src.data(), src.size());
		return src.size();
	}

	size_t File::read(unsigned char* dst, size_t count) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		mFileStream->read(reinterpret_cast<char*>(dst), count);
		auto c = getLastReadCount();
		ASSERT(count == c) << "Failed to read " << (count - c) << " bytes (" << c << "/" << count << ")";
		return c;
	}

	size_t File::write(const unsigned char* src, size_t count) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		mFileStream->write(reinterpret_cast<const char*>(src), count);
		return count;
	}

	size_t File::read(std::vector<unsigned char>& dst) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		auto size = fileSize();
		dst.resize(size);
		mFileStream->read(reinterpret_cast<char*>(dst.data()), size);
		auto c = getLastReadCount();
		ASSERT(size == c) << "Failed to read " << (size - c) << " bytes (" << c << "/" << size << ")";
		return c;
	}

	size_t File::write(const std::vector<unsigned char>& src) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}
		mFileStream->write(reinterpret_cast<const char*>(src.data()), src.size());
		return src.size();
	}

	size_t File::read(std::stringstream& dst, size_t count) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}

		std::array<char, 2048> buf;
		size_t readCount = 0;
		for (;;) {
			mFileStream->read(buf.data(), count > buf.size() ? buf.size() : count);
			size_t c = getLastReadCount();
			if (c == 0) {
				break;
			}
			dst.write(buf.data(), c);
			readCount += c;
			count -= c;
		}
		return readCount;
	}

	size_t File::write(std::stringstream& src, size_t count) {
		if (!mFileStream) {
			LOG(LogWarning) << "File not open:" << mFilePath;
			ASSERT(mFileStream.has_value()) << "File not open:" << mFilePath;
			return 0;
		}

		std::array<char, 2048> buf;
		size_t writeCount = 0;
		for (;;) {
			src.read(buf.data(), count > buf.size() ? buf.size() : count);
			size_t c = static_cast<size_t>(src.gcount());
			if (c <= 0) {
				break;
			}

			mFileStream->write(buf.data(), c);
			writeCount += c;
			count -= c;
		}
		return writeCount;
	}
}
}
