#include "audio/AudioUtils.h"

#include "logging/LoggingAll.h"
#include "math/MathAll.h"

#include <math.h>

namespace l::audio {

    float GetFrequencyFromNote(float note) {
        return 440.0f * powf(2.0f, (note - 49.0f) / 12.0f);
    }

	float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<void(int32_t, int32_t, bool)> process, std::function<void(float)> update) {
		float startNum = static_cast<float>(start);
		while (startNum < static_cast<float>(end)) {
			bool updated = false;
			if (samplesLeft < 1.0f) {
				samplesLeft += updateSamples;
				if (update != nullptr) {
					update(startNum);
				}
				updated = true;
			}

			// Update stuff
			// process samples until 
			// * batch max has been reached
			// * end has been reached

			float samples = l::math::functions::min(static_cast<float>(end) - startNum, samplesLeft);
			if (process != nullptr) {
				process(static_cast<int32_t>(startNum), static_cast<int32_t>(startNum + samples), updated);
			}
			startNum += samples;
			samplesLeft -= samples;
		}
		return samplesLeft;
	}

}
