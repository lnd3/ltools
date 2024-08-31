#include "hid/KeyboardPiano.h"

#include <memory>

namespace l::hid {

	void KeyboardPiano::SetKeyState(KeyState* keyState) {
		mKeyState = keyState;
	}
	
	void KeyboardPiano::SetNoteProcessor(INoteProcessor* notePlayer) {
		mNotePlayer = notePlayer;
	}

	void KeyboardPiano::ForEachNoteChange(std::function<void(int32_t, bool)> noteHandler) {
		mKeyState->ForEachKeyChange([&](int32_t keyCode, bool pressedNow, bool releasedNow) {
			if (pressedNow) {
				if (keyCode == mKeyFunctions[KeyFunctionTypes::OCTAVE_DOWN]) {
					noteHandler(0, false);
					mOctave -= 1;
					mOctave = mOctave < -5 ? -5 : mOctave;
					mOctave = mOctave > 10 ? 10 : mOctave;
				}
				else if (keyCode == mKeyFunctions[KeyFunctionTypes::OCTAVE_UP]) {
					noteHandler(0, false);
					mOctave += 1;
					mOctave = mOctave < -5 ? -5 : mOctave;
					mOctave = mOctave > 10 ? 10 : mOctave;
				}
				else {
					int32_t note = ConvertKeyCodeToNote(keyCode, mOctave);
					if (note > 0) {
						noteHandler(note, true);
					}
				}
			}
			if (releasedNow) {
				int32_t note = ConvertKeyCodeToNote(keyCode, mOctave);
				if (note > 0) {
					noteHandler(note, false);
				}
			}
			});
	}

	void KeyboardPiano::Update() {
		if (mKeyState == nullptr || mNotePlayer == nullptr) {
			return;
		}

		mKeyState->ForEachKeyChange([&](int32_t keyCode, bool pressedNow, bool releasedNow) {
			if (pressedNow) {
				if (keyCode == mKeyFunctions[KeyFunctionTypes::OCTAVE_DOWN]) {
					mNotePlayer->NoteOff();
					mOctave -= 1;
					mOctave = mOctave < -5 ? -5 : mOctave;
					mOctave = mOctave > 10 ? 10 : mOctave;
				}
				else if (keyCode == mKeyFunctions[KeyFunctionTypes::OCTAVE_UP]) {
					mNotePlayer->NoteOff();
					mOctave += 1;
					mOctave = mOctave < -5 ? -5 : mOctave;
					mOctave = mOctave > 10 ? 10 : mOctave;
				}
				else {
					int32_t note = ConvertKeyCodeToNote(keyCode, mOctave);
					if (note > 0) {
						mNotePlayer->NoteOn(note);
					}
				}
			}
			if (releasedNow) {
				int32_t note = ConvertKeyCodeToNote(keyCode, mOctave);
				if (note > 0) {
					mNotePlayer->NoteOff(note);
				}
			}

			});
	}

	int32_t KeyboardPiano::ConvertKeyCodeToNote(int32_t keyCode, int32_t octave) {
		auto key = mKeyCodeToNote[keyCode];
		if (key > 0) {
			return key + octave * 12;
		}
		return 0;
	}

	int32_t KeyboardPiano::ConvertCharCodeToNote(int32_t charCode, int32_t octave) {
		auto key = mCharCodeToNote[charCode];
		if (key > 0) {
			return key + octave * 12;
		}
		return 0;
	}

	void KeyboardPiano::MapKeyFunctions(KeyFunctionTypes function, int32_t keyCode) {
		mKeyFunctions.emplace(function, keyCode);
	}


}
