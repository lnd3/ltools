#pragma once

#include <string>
#include <ctime>
#include <sstream>
#include <vector>
#include <climits>

namespace l {
namespace filesystem {
	class IFile {
	public:
		IFile() = default;
		virtual ~IFile() = default;

		virtual IFile& modeBinary() = 0;
		virtual IFile& modeRead() = 0;
		virtual IFile& modeReadPreload() = 0;
		virtual IFile& modeWrite() = 0;
		virtual IFile& modeWriteTrunc() = 0;

		virtual bool open() = 0;
		virtual void close() = 0;
		virtual void flush() = 0;

		virtual std::time_t lastWriteTime() = 0;
		virtual std::string lastWriteAsString() = 0; // format: yyyy-mm-dd hh:mm
		virtual std::size_t fileSize() = 0;

		virtual void setReadPosition(size_t position) = 0;
		virtual void setWritePosition(size_t position) = 0;
		virtual size_t getReadPosition() = 0;
		virtual size_t getWritePosition() = 0;
		virtual size_t getLastReadCount() = 0;

		virtual size_t read(char* dst, size_t count = INT_MAX) = 0;
		virtual size_t write(const char* src, size_t count = INT_MAX) = 0;
		virtual size_t read(std::vector<char>& dst) = 0;
		virtual size_t write(const std::vector<char>& src) = 0;

		virtual size_t read(unsigned char* dst, size_t count = INT_MAX) = 0;
		virtual size_t write(const unsigned char* src, size_t count = INT_MAX) = 0;
		virtual size_t read(std::vector<unsigned char>& dst) = 0;
		virtual size_t write(const std::vector<unsigned char>& src) = 0;

		virtual size_t read(std::stringstream& dst, size_t count = INT_MAX) = 0;
		virtual size_t write(std::stringstream& src, size_t count = INT_MAX) = 0;
	};
}
}
