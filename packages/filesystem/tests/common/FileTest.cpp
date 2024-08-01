#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "filesystem/file.h"

using namespace l;

TEST(File, Init) {

	std::string content = "test123";
	{
		l::filesystem::File file("./tests/test.txt");
		file.modeWrite();
		file.open();

		std::stringstream buf;

		buf << content;

		file.write(buf);
		file.close();
	}

	{
		l::filesystem::File file("./tests/test.txt");
		file.modeReadPreload();
		file.open();

		std::stringstream buf;

		file.read(buf);
		file.close();

		std::string readContent;
		buf >> readContent;

		TEST_TRUE(readContent == content, "");
	}


	return 0;
}
