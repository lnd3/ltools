
#include "math/MathTween.h"
#include "math/MathSmooth.h"
#include "math/MathFunc.h"

namespace l::math::tween {

	float GetRWAFactor(int32_t steps, float limit) {
		return 1.0f - l::math::functions::pow(l::math::constants::E_f, l::math::functions::log(limit) / static_cast<float>(steps));
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

	void DynamicTween::Reset(float value, bool ignoreDynamicSmooth) {
		mTargetValue = value;
		mTargetValuePrev = value;
		if (ignoreDynamicSmooth) {
			mValue = value;
		}
		mCounter = 0;
	}

	void DynamicTween::SetTweenLength(int32_t tweenCount) {
		mUpdateCount = l::math::functions::max(tweenCount, 4);
	}

	void DynamicTween::SetTarget(float target, int32_t tweenCount) {
		mTargetValuePrev = mValue;
		mTargetValue = target;
		mCounter = 0;
		mUpdateCount = l::math::functions::max(tweenCount, 4);
	}

	// update rate in audio processing, the number of samples between calls
	// we can use this to update expensive interpolation here
	void DynamicTween::Update() {
		mTarget = mTargetValue;
		if (mCounter < mUpdateCount) {
			float t = mCounter / static_cast<float>(mUpdateCount);
			mTarget = mTargetValuePrev + l::math::smooth::smoothPolyh3(t) * (mTargetValue - mTargetValuePrev);
		}
	}

	float DynamicTween::Next() {
		mValue += mDynamicSmooth * (mTarget - mValue);
		mCounter++;
		return mValue;
	}

	float DynamicTween::Value() {
		mValue += mDynamicSmooth * (mTarget - mValue);
		return mValue;
	}
}
