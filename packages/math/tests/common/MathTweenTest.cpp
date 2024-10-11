#include "testing/Test.h"
#include "logging/Log.h"

#include "math/MathAll.h"

using namespace l;
using namespace l::math::tween;

TEST(MathTween, RecentWeightedAverage) {
	RecentWeightedAverage rwa;

	rwa.SetConvergence(GetRWAFactor(10, 0.1f));
	rwa.SetValue(1.0f);
	rwa.SetTarget(0.0f);
	TEST_FUZZY(rwa.Value(), 1.0f, 0.1f, "");
	for (int i = 0; i < 10; i++) {
		auto value = rwa.Next();
		LOG(LogInfo) << "RWA: " << value;
	}
	TEST_FUZZY(rwa.Value(), 0.0f, 0.1f, "");

	return 0;
}

TEST(MathTween, DynamicTween) {
	DynamicTween tween(0.25f);
	tween.SetValue(1.0f);
	tween.SetTarget(0.0, 10);
	TEST_FUZZY(tween.Value(), 1.0f, 0.1f, "");
	for (int i = 0; i < 10; i++) {
		tween.Update();
		auto value = tween.Next();
		LOG(LogInfo) << "Tween: " << value;
	}
	TEST_FUZZY(tween.Value(), 0.0f, 0.01f, "");

	return 0;
}

TEST(MathTween, DynamicTweenVeryShortBatch) {
	DynamicTween tween(0.01f);
	tween.SetValue(1.0f);
	tween.SetTarget(0.0, 22);
	TEST_FUZZY(tween.Value(), 1.0f, 0.01f, "");
	for (int i = 0; i < 30; i++) {
		if (i % 10 == 0) {
			tween.Update(10);
		}
		auto value = tween.Next();
		LOG(LogInfo) << "Tween (" << i << "): " << value;
	}
	TEST_FUZZY(tween.Value(), 0.0f, 0.01f, "");

	return 0;
}

TEST(MathTween, DynamicTweenShortBatch) {
	DynamicTween tween(0.135f);
	tween.SetValue(1.0f);
	tween.SetTarget(0.0, 102);
	TEST_FUZZY(tween.Value(), 1.0f, 0.01f, "");
	for (int i = 0; i < 110; i++) {
		if (i % 10 == 0) {
			tween.Update(10);
		}
		auto value = tween.Next();
		LOG(LogInfo) << "Tween (" << i << "): " << value;
	}
	TEST_FUZZY(tween.Value(), 0.0f, 0.01f, "");

	return 0;
}

TEST(MathTween, DynamicTweenLongBatch) {
	float updateRate = 100.0f;
	DynamicTween tween(0.35f);
	tween.SetValue(1.0f);
	tween.SetTarget(0.0, 1020);
	TEST_FUZZY(tween.Value(), 1.0f, 0.01f, "");
	for (int i = 0; i < 1100; i++) {
		if (i % 10 == 0) {
			tween.Update(updateRate);
		}
		auto value = tween.Next();
		LOG(LogInfo) << "Tween (" << i << "): " << value;
	}
	TEST_FUZZY(tween.Value(), 0.0f, 0.02f, "");

	return 0;
}

TEST(MathTween, Math) {
	float t = 1.0f;
	float v = l::math::min2(0.0f, t);
	TEST_FUZZY(0.0f, v, 0.0001f, "");
	return 0;
}