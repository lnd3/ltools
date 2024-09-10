#pragma once

#include "logging/LoggingAll.h"

#include "hid/KeyState.h"

#include <functional>
#include <unordered_map>
#include <array>

namespace l::hid {

	class INoteProcessor {
	public:
		virtual ~INoteProcessor() = default;
		virtual void NoteOn(int32_t, int32_t = 127) {}
		virtual void NoteOff() {}
		virtual void NoteOff(int32_t) {}
		virtual void NoteSustain(bool) {}
	};

	enum class KeyFunctionTypes {
		OCTAVE_DOWN = 0,
		OCTAVE_UP
	};

	class KeyboardPiano {
	public:
		const std::array<int32_t, 19> keyMapFromNoteBUpper = {49, 81, 50, 87, 51, 69, 82, 53, 84, 54, 89, 55, 85, 73, 57, 79, 48, 80, 43};
		const std::array<int32_t, 16> keyMapFromNoteBLower = {65, 90, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77, 44, 76, 46};
		const std::array<char, 18> charMapFromNoteBUpper = { '1', 'q', '2', 'w', '3', 'e', 'r', '5', 't', '6', 'y', '7', 'u', 'i', '9', 'o', '0', 'p' };

		KeyboardPiano() : KeyboardPiano(nullptr, nullptr) {}
		KeyboardPiano(l::hid::KeyState* keyState, l::hid::INoteProcessor* instrument) :
			mKeyState(keyState),
			mNotePlayer(instrument)
		{
			mKeyFunctions[KeyFunctionTypes::OCTAVE_DOWN] = 291; // F2
			mKeyFunctions[KeyFunctionTypes::OCTAVE_UP] = 292; // F3

			int32_t i = 0;
			int32_t key ;
			for (i = 0; i < static_cast<int32_t>(keyMapFromNoteBLower.size()); i++) {
				key = keyMapFromNoteBLower[i];
				mKeyCodeToNote[key] = i - 12;
			}
			for (i = 0; i < static_cast<int32_t>(keyMapFromNoteBUpper.size()); i++) {
				key = keyMapFromNoteBUpper[i];
				mKeyCodeToNote[key] = i;
			}
			for (i = 0; i < static_cast<int32_t>(charMapFromNoteBUpper.size()); i++) {
				char c = charMapFromNoteBUpper[i];
				mCharCodeToNote[c] = i;
			}
		}

		void SetKeyState(KeyState* keyState);
		void SetNoteProcessor(INoteProcessor* notePlayer);
		void ForEachNoteChange(std::function<void(int32_t, bool)> noteHandler);

		void Update();

		int32_t ConvertKeyCodeToNote(int32_t keyCode, int32_t octave = 3);
		int32_t ConvertCharCodeToNote(int32_t charCode, int32_t octave = 3);

		void MapKeyFunctions(KeyFunctionTypes function, int32_t keyCode);

	protected:
		int32_t mOctave = 3;
		std::unordered_map<int32_t, int32_t> mKeyCodeToNote;
		std::unordered_map<int32_t, int32_t> mCharCodeToNote;
		std::unordered_map<KeyFunctionTypes, int32_t> mKeyFunctions;
		KeyState* mKeyState = nullptr;
		INoteProcessor* mNotePlayer = nullptr;
	};

}
