#include "testing/Test.h"
#include "audio/AudioUtils.h"

#include <thread>


TEST(AudioUtils, Basic) {

	l::audio::PCBeep(800, 25);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return 0;
}


