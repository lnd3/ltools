#pragma once

#include "IFile.h"

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <optional>
#include <chrono>
#include <sstream>
#include <fstream>

namespace l {
namespace filesystem {

	template<class T, const size_t SIZE>
	void convert(std::stringstream& dst, std::array<T, SIZE>& src, size_t count = 0) {
		static_assert(sizeof(T) == sizeof(char));
		dst.write(reinterpret_cast<char*>(src.data()), count > 0 ? count : src.size());
	}

	bool read(const std::filesystem::path& mLocation, char* dst, size_t count, size_t position = 0);
	bool write(const std::filesystem::path& mLocation, const char* src, size_t count, size_t position = 0);

	bool read(const std::filesystem::path& mLocation, std::stringstream& data);
	bool write(const std::filesystem::path& mLocation, std::stringstream& data);

	std::time_t getTime(std::filesystem::file_time_type tp);
	std::string toString(std::time_t t, std::string format = "%Y-%m-%d %X");

	class File : public IFile {
	public:
		File(std::filesystem::path filePath);
		virtual ~File();

		virtual File& modeBinary();
		virtual File& modeRead();
		virtual File& modeReadPreload();
		virtual File& modeWrite();
		virtual File& modeWriteTrunc();

		virtual bool open();
		virtual void close();
		virtual void flush();

		virtual std::time_t lastWriteTime();
		virtual std::string lastWriteAsString(); // format: yyyy-mm-dd hh:mm
		virtual std::size_t fileSize();

		virtual void setReadPosition(size_t position);
		virtual void setWritePosition(size_t position);
		virtual size_t getReadPosition();
		virtual size_t getWritePosition();
		virtual size_t getLastReadCount();

		virtual size_t read(char* dst, size_t count = INT_MAX);
		virtual size_t write(const char* src, size_t count = INT_MAX);
		virtual size_t read(std::vector<char>& dst);
		virtual size_t write(const std::vector<char>& src);

		virtual size_t read(unsigned char* dst, size_t count = INT_MAX);
		virtual size_t write(const unsigned char* src, size_t count = INT_MAX);
		virtual size_t read(std::vector<unsigned char>& dst);
		virtual size_t write(const std::vector<unsigned char>& src);

		virtual size_t read(std::stringstream& dst, size_t count = INT_MAX);
		virtual size_t write(std::stringstream& src, size_t count = INT_MAX);
	protected:
		std::filesystem::path mFilePath;
		int mFileMode;
		std::optional<std::fstream> mFileStream;
	};
}
}
