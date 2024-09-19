#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include <optional>
#include <string>

namespace l::math::tween {

    float GetRWAFactor(int32_t steps, float limit);

    class RecentWeightedAverage {
    public:
        RecentWeightedAverage() = default;
        ~RecentWeightedAverage() = default;

        void Reset(float value);
        void SetValue(float value);
        void SetConvergence(float smooth);
        void SetTarget(float target);
        float Next();
        float Value();
    protected:
        float mSmooth;
        float mValue;
        float mTargetValue;
    };

    /**********************************************************************************/

    class DynamicTween {
    public:
        DynamicTween(float dynamicSmoothAccuracy = 0.35f) :
            mDynamicSmoothAccuracy(dynamicSmoothAccuracy)
        {}
        ~DynamicTween() = default;

        void Reset(float value);
        void SetValue(float value, bool setSmooth = true);
        void SetTweenLength(int32_t tweenCount);
        void SetTarget(float target, int32_t tweenCount = 0);
        void Update(float updateRate = 1.0f);
        float Next();
        float Value();

    protected:
        float mUpdateRate = 0.0f;
        int32_t mUpdateCount = 1;
        float mDynamicSmoothAccuracy = 0.1f;
        float mDynamicSmooth = 0.25f;

        float mValue = 0.0f;
        float mTarget = 0.0f;
        float mTargetValue = 0.0f;
        float mTargetValuePrev = 0.0f;
        int32_t mCounter = 0;
    };
}
