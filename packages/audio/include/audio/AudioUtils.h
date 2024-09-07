#pragma once

#include <functional>

namespace l::audio {
    float GetFrequencyFromNote(float note);
    double GetPhaseModifier(double note, double modifier);
    float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<void()> update, std::function<void(int32_t, int32_t, bool)> process);
}
