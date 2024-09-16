#include "audio/AudioUtils.h"

#include "logging/LoggingAll.h"
#include "math/MathAll.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::audio {
	const float gNoNote_f = -500.0f;
	const int32_t gNoNote = -500;

    float GetFrequencyFromNote(float note) {
        return 440.0f * l::math::functions::pow(2.0f, (note - 49.0f) / 12.0f);
    }

	double GetPhaseModifier(double note, double modifier) {
		double limit = 1.0 / l::math::functions::max(note / 25.0, 1.0);
		return 800.0 * modifier * modifier * limit;
	}

	int32_t GetSamplesFromMS(float ms, float rate) {
		return static_cast<int32_t>(ms * rate / 1000.0f);
	}

	float GetMSFromSamples(float numSamples, float rate) {
		return numSamples * 1000.0f / rate;
	}

	float GetRWAFactorFromMS(float ms, float limit, float rate) {
		int32_t samples = static_cast<int32_t>(ms * rate / 1000.0f);
		return GetRWAFactorFromSamples(samples, limit);
	}

	float GetRWAFactorFromMSAttackSkew(float ms, float limit, float rate) {
		float msRoot = l::math::functions::sqrt(ms);
		int32_t samples = static_cast<int32_t>(msRoot * rate / 1000.0f);
		float factor = l::audio::GetRWAFactorFromSamples(samples, limit);
		factor *= factor;
		return factor;
	}

	float GetRWAFactorFromSamples(int32_t numSamples, float limit) {
		return 1.0f - l::math::functions::pow(l::math::constants::E_f, l::math::functions::log(limit) / static_cast<float>(numSamples));
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
