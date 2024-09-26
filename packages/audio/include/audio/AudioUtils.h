#pragma once

#include "math/MathAll.h"

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

    int32_t GetAudioTicksFromMS(float ms, float sampleRate = 44100.0f);
    float GetMSFromAudioTicks(float numTicks, float sampleRate = 44100.0f);

    float GetRWAFactorFromMS(float numMS, float limit = 0.0001f, float rwaUpdateRate = 1.0f, float sampleRate = 44100.0f);
    float GetRWAFactorFromMSSkewed(float ms, float limit = 0.0001f, float rwaUpdateRate = 1.0f, float sampleRate = 44100.0f);
        

    float BatchUpdate(float updateSamples, float samplesLeft, int32_t start, int32_t end, std::function<float()> update, std::function<void(int32_t, int32_t, bool)> process);

    template<class T>
    class FilterRWA {
    public:

        FilterRWA() :
            mSmooth(static_cast<T>(0.005)),
            mValue(static_cast<T>(0)),
            mTargetValue(static_cast<T>(0)),
            mRWAUpdateRate(static_cast<T>(1.0))
        {}
        ~FilterRWA() = default;

        bool SnapAt(T value = static_cast<T>(0), T proximity = static_cast<T>(0.00001)) {
            if (l::math::functions::abs(mValue - value) < proximity) {
                mValue = value;
                return true;
            }
            return false;
        }

        FilterRWA<T>& SetConvergenceInMs(T convergenceInMS, T limit = static_cast<T>(0.0001), T sampleRate = static_cast<T>(44100.0)) {
            mSmooth = GetRWAFactorFromMS(convergenceInMS, limit, mRWAUpdateRate, sampleRate);
            return *this;
        }

        FilterRWA<T>& SetConvergenceFactor(T smooth = static_cast<T>(0.005)) {
            mSmooth = smooth;
            return *this;
        }

        FilterRWA<T>& SetConvergenceInTicks(T ticks, T limit = static_cast<T>(0.001)) {
            mSmooth = l::math::tween::GetRWAFactor(static_cast<int32_t>(ticks), limit);
            return *this;
        }

        FilterRWA<T>& SetRWAUpdateRate(T fetchesPerUpdate = static_cast<T>(1.0)) {
            mRWAUpdateRate = l::math::functions::max2(fetchesPerUpdate, static_cast<T>(1.0));
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

        T& Value() {
            return mValue;
        }

    protected:
        T mSmooth;
        T mValue;
        T mTargetValue;
        T mRWAUpdateRate;
    };

    using FilterRWAFloat = FilterRWA<float>;

}
