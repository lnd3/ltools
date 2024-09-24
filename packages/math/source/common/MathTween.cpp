
#include "math/MathTween.h"
#include "math/MathSmooth.h"
#include "math/MathFunc.h"

namespace l::math::tween {

	float GetRWAFactor(int32_t steps, float limit) {
		if (steps < 2) {
			return 1.0f;
		}
		return 1.0f - l::math::functions::pow(l::math::constants::E_f, l::math::functions::log(limit) / static_cast<float>(steps-1));
	}

	void RecentWeightedAverage::Reset(float value) {
		mTargetValue = value;
		mValue = value;
	}

	void RecentWeightedAverage::SetValue(float value) {
		mValue = value;
	}

	void RecentWeightedAverage::SetConvergence(float smooth) {
		mSmooth = smooth;
	}

	void RecentWeightedAverage::SetTarget(float target) {
		mTargetValue = target;
	}

	float RecentWeightedAverage::Next() {
		mValue += mSmooth * (mTargetValue - mValue);
		return mValue;
	}

	float RecentWeightedAverage::Value() {
		return mValue;
	}

	/**********************************************************************************/

	void DynamicTween::Reset(float value) {
		mTargetValue = value;
		mTargetValuePrev = value;
		mTarget = value;
		mValue = value;
		mCounter = 0;
	}

	void DynamicTween::SetValue(float value, bool setSmooth) {
		mTargetValue = value;
		mTargetValuePrev = value;
		if (setSmooth) {
			mTarget = value;
			mValue = value;
		}
	}

	void DynamicTween::SetTweenLength(int32_t tweenCount) {
		mUpdateCount = l::math::functions::max(tweenCount, 4);
	}

	void DynamicTween::SetTarget(float target, int32_t tweenCount) {
		mTargetValuePrev = mValue;
		mTargetValue = target;
		mCounter = 0;
		if (tweenCount >= 4) {
			mUpdateCount = l::math::functions::max(tweenCount, 4);
		}
	}

	// update rate in audio processing, the number of samples between calls
	// we can use this to update expensive interpolation here
	void DynamicTween::Update(float updateRate) {
		mDynamicSmooth = GetRWAFactor(static_cast<int32_t>(updateRate), mDynamicSmoothAccuracy);

		mTarget = mTargetValue;
		if (mCounter < mUpdateCount) {
			float t = (mCounter + 0.5f) / static_cast<float>(mUpdateCount - 0.5f);
			t = l::math::functions::clamp(t, 0.0f, 1.0f);
			mTarget = mTargetValuePrev + l::math::smooth::smoothPolyh3(t) * (mTargetValue - mTargetValuePrev);
		}
	}

	float DynamicTween::Next() {
		mValue += mDynamicSmooth * (mTarget - mValue);
		mCounter++;
		return mValue;
	}

	float DynamicTween::Value() {
		return mValue;
	}

}
