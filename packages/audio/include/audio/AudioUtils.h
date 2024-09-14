#pragma once

#include "math/MathFunc.h"

#include <functional>

namespace l::audio {

    class INoteProcessor {
    public:
        virtual ~INoteProcessor() = default;
        virtual void NoteOn(int32_t, int32_t = 127) {}
        virtual void NoteOff() {}
        virtual void NoteOff(int32_t) {}
        virtual void NoteSustain(bool) {}
    };

    extern const float gNoNote_f;
    extern const int32_t gNoNote;
    float GetFrequencyFromNote(float note);
    double GetPhaseModifier(double note, double modifier);
    float GetRCAFactor(float numSamples, float limit = 0.01f);
    float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<void()> update, std::function<void(int32_t, int32_t, bool)> process);

    template<class T>
    class FilterRWA {
    public:

        FilterRWA() :
            mSmooth(static_cast<T>(0.005)),
            mValue(static_cast<T>(0)),
            mTargetValue(static_cast<T>(0))
        {}
        ~FilterRWA() {}

        bool SnapAt(T value = static_cast<T>(0), T proximity = static_cast<T>(0.00001)) {
            if (l::math::functions::abs(mValue - value) < proximity) {
                mValue = value;
                return true;
            }
            return false;
        }

        FilterRWA<T>& SetConvergence(T smooth = static_cast<T>(0.005)) {
            mSmooth = smooth;
            return *this;
        }

        FilterRWA<T>& SetTarget(T target) {
            mTargetValue = target;
            return *this;
        }

        T Next() {
            mValue += mSmooth * (mTargetValue - mValue);
            return mValue;
        }

    protected:
        T mSmooth;
        T mValue;
        T mTargetValue;
    };
}
