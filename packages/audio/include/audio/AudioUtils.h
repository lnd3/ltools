#pragma once

#include <functional>

namespace l::audio {
    extern const float gNoNote_f;
    extern const int32_t gNoNote;
    float GetFrequencyFromNote(float note);
    double GetPhaseModifier(double note, double modifier);
    float GetRCAFactor(float numSamples, float limit = 0.01f);
    float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<void()> update, std::function<void(int32_t, int32_t, bool)> process);
}
