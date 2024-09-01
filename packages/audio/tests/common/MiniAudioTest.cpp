#include "testing/Test.h"

#include "audio/Audio.h"

#include <thread>


TEST(MiniAudio, Setup) {

	if (!l::audio::Init()) {
		return 1;
	}



	std::this_thread::sleep_for(std::chrono::milliseconds(100));


	l::audio::Deinit();

	return 0;
}
