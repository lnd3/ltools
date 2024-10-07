#include "audio/AudioUtils.h"

#include "logging/LoggingAll.h"
#include "math/MathAll.h"

#include "math/MathFunc.h"
#include "math/MathTween.h"

#include <math.h>


namespace l::audio {
#ifdef BSYSTEM_PLATFORM_Windows
#include <Windows.h>
	void PCBeep(int32_t freq, int32_t duration) {
		Beep(freq, duration);
	}
#else
#include <stdio.h>
	void PCBeep(int32_t, int32_t) {
		system("echo -e "\007" >/dev/tty10");
	}
#endif
}

namespace l::audio {
	const float gNoNote_f = -500.0f;
	const int32_t gNoNote = -500;

    float GetFrequencyFromNote(float note) {
        return 440.0f * l::math::functions::pow(2.0f, (note - 49.0f) / 12.0f);
    }

	double GetPhaseModifier(double note, double modifier) {
		double limit = 1.0 / l::math::functions::max2(note / 25.0, 1.0);
		return 800.0 * modifier * modifier * limit;
	}

	int32_t GetAudioTicksFromMS(float ms, float sampleRate) {
		return static_cast<int32_t>(ms * sampleRate / 1000.0f);
	}

	float GetMSFromAudioTicks(float numAudioSamples, float sampleRate) {
		return numAudioSamples * 1000.0f / sampleRate;
	}

	float GetRWAFactorFromMS(float ms, float limit, float rwaUpdateRate, float sampleRate) {
		int32_t updateSteps = GetAudioTicksFromMS(ms, sampleRate / rwaUpdateRate);
		return l::math::tween::GetRWAFactor(updateSteps, limit);
	}

	float GetRWAFactorFromMSSkewed(float ms, float limit, float rwaUpdateRate, float sampleRate) {
		float msRoot = l::math::functions::sqrt(ms);
		int32_t steps = GetAudioTicksFromMS(msRoot, sampleRate / rwaUpdateRate);
		float factor = l::math::tween::GetRWAFactor(steps, limit);
		factor *= factor;
		return factor;
	}

	float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<float()> update, std::function<void(int32_t, int32_t, bool)> process) {
		updateSamples = l::math::functions::max2(updateSamples, 1.0f);
		float startNum = static_cast<float>(start);
		while (startNum < static_cast<float>(end)) {
			bool updated = false;
			if (samplesLeft < 1.0f) {
				samplesLeft += updateSamples;
				if (update != nullptr) {
					float rate = update();
					updateSamples = rate > 0.0f ? rate : updateSamples;
				}
				updated = true;
			}

			// Update stuff
			// process samples until 
			// * batch max has been reached
			// * end has been reached

			float samples = l::math::functions::min2(static_cast<float>(end) - startNum, samplesLeft);
			if (process != nullptr) {
				process(static_cast<int32_t>(startNum), static_cast<int32_t>(startNum + samples), updated);
			}
			startNum += samples;
			samplesLeft -= samples;
		}
		return samplesLeft;
	}
}
