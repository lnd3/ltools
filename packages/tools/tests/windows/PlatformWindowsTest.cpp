#include "testing/Test.h"

#include "tools/platform/Platform.h"

#include <filesystem>
#include <memory>

using namespace l;



TEST(PlatformWindows, FileSystem) {

	int argc = 1;
	const char* argv[1];
	argv[0] = R"(./Debug/TestStorage.exe)";


	platform::Cmd::ParseArguments(argc, argv);
	std::filesystem::path app(platform::Cmd::GetCommandLineArgument(0));
	std::cout << app << std::endl;

	auto s = platform::FS::GetAppDataPath();
	std::cout << string::narrow(s) << std::endl;
	auto s2 = platform::FS::GetProgramPath();
	std::cout << string::narrow(s2) << std::endl;

	return 0;
}
