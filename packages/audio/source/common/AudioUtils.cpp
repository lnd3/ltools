#include "audio/AudioUtils.h"

#include "logging/LoggingAll.h"
#include "math/MathAll.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::audio {

    float GetFrequencyFromNote(float note) {
        return 440.0f * l::math::functions::pow(2.0f, (note - 49.0f) / 12.0f);
    }

	double GetPhaseModifier(double note, double modifier) {
		double limit = 1.0 / l::math::functions::max(note / 25.0, 1.0);
		return 800.0 * modifier * modifier * limit;
	}

	float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<void()> update, std::function<void(int32_t, int32_t, bool)> process) {
		float startNum = static_cast<float>(start);
		while (startNum < static_cast<float>(end)) {
			bool updated = false;
			if (samplesLeft < 1.0f) {
				samplesLeft += updateSamples;
				if (update != nullptr) {
					update();
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
