#include "hid/KeyState.h"

#include <memory>

namespace l::hid {

	void KeyPressState::UpdateKeyDown() {
		pressed = true;

		releasedPrev = released;
		released = false;
	}

	void KeyPressState::UpdateKeyUp() {
		releasedPrev = released;
		released = true;
	}

	void KeyPressState::UpdateFrameState() {
		pressedNow = false;
		releasedNow = false;
		if (pressed && !pressedPrev) {
			pressedNow = true;
		}
		if (released && !releasedPrev) {
			releasedNow = true;
			pressed = false;
		}
		pressedPrev = pressed;
		releasedPrev = released;

		//if (pressedNow || releasedNow || pressed) {
		//	LOG(LogInfo) << "pressed now: " << pressedNow << ", released now: " << releasedNow << ", pressed: " << pressed;
		//}
	}

	bool KeyPressState::IsReleased() {
		return !pressed;
	}

	bool KeyPressState::IsPressed() {
		return pressed;
	}

	bool KeyPressState::IsReleasedNow() {
		return releasedNow;
	}

	bool KeyPressState::IsPressedNow() {
		return pressedNow;
	}

	void KeyState::UpdateKeyDown(int32_t keyCode) {
		mActiveKeys[keyCode].UpdateKeyDown();
	}

	void KeyState::UpdateKeyUp(int32_t keyCode) {
		// Up key event may happen if press was made out of focus so it may 
		// be null in this situation. This means the up key event is invalid.
		mActiveKeys[keyCode].UpdateKeyUp();
	}

	bool KeyState::IsReleased(int32_t keyCode) {
		return mActiveKeys[keyCode].IsReleased();
	}

	bool KeyState::IsPressed(int32_t keyCode) {
		return mActiveKeys[keyCode].IsPressed();
	}

	bool KeyState::IsReleasedNow(int32_t keyCode) {
		return mActiveKeys[keyCode].IsReleasedNow();
	}

	bool KeyState::IsPressedNow(int32_t keyCode) {
		return mActiveKeys[keyCode].IsPressedNow();
	}

	void KeyState::UpdateFrameState() {
		for (auto& key : mActiveKeys) {
			key.second.UpdateFrameState();
		}
	}

	void KeyState::ForEachKeyChange(std::function<void(int32_t, bool, bool)> keyHandler) {
		for (auto& key : mActiveKeys) {
			auto& keyState = key.second;
			bool pressedNow = keyState.IsPressedNow();
			bool releasedNow = keyState.IsReleasedNow();
			if (pressedNow || releasedNow) {
				keyHandler(key.first, pressedNow, releasedNow);
			}
		}
	}

	void KeyState::ForEachKey(std::function<void(int32_t, KeyPressState&)> keyHandler) {
		for (auto& key : mActiveKeys) {
			keyHandler(key.first, key.second);
		}
	}

}
