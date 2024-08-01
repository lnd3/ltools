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
		std::ios_base::openmode mFileMode;
		std::filesystem::path mFilePath;
		std::optional<std::fstream> mFileStream;
	};
}
}
