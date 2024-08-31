#pragma once

#include "logging/LoggingAll.h"

#include <unordered_map>

namespace l::hid {

	class KeyPressState {
	public:
		KeyPressState() = default;

		void UpdateKeyDown();
		void UpdateKeyUp();
		void UpdateFrameState();

		bool IsReleased();
		bool IsPressed();
		bool IsReleasedNow();
		bool IsPressedNow();

	protected:
		bool pressed = false;
		bool pressedPrev = false;

		bool released = true;
		bool releasedPrev = true;

		bool pressedNow = false;
		bool releasedNow = false;
	};

	class KeyState {
	public:
		KeyState() {
			mActiveKeys.clear();
		}
		~KeyState() = default;

		void UpdateKeyDown(int32_t keyCode);
		void UpdateKeyUp(int32_t keyCode);
		bool IsReleased(int32_t keyCode);
		bool IsPressed(int32_t keyCode);
		bool IsReleasedNow(int32_t keyCode);
		bool IsPressedNow(int32_t keyCode);
		void UpdateFrameState();
		void ForEachKeyChange(std::function<void(int32_t, bool, bool)> keyHandler);
		void ForEachKey(std::function<void(int32_t, KeyPressState&)> keyHandler);
	protected:
		std::unordered_map<int32_t, KeyPressState> mActiveKeys;
	};
}
