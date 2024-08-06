#pragma once

namespace l {
namespace x {

namespace string
{
	typedef struct SString {
		char* str;
		int cur_len;
		int max_len;
	} SString;

	SString CreateString(char* src, int len);
	SString CreateString(const char* src);
	void EndString(SString& dst);
	void Append(SString& dst, const char src);

	void Append(SString& dst, const char* src);
	void Append(SString& dst, int value);
	void Append(SString& dst, SString& src);

	bool Equals(const SString& src0, const SString& src1);
	bool Equals(const SString& src0, const char* src1);

	int StrLen(SString& dst);


	int StrLen(const char* src, int max_chars = 50);
	void StrCpy(char* dst, const char* src, int max_chars = 50);

	void SetCharAt(char* string, int at, char c);

	int Append(char* dst, const char* src);
	int Append(char* dst, int value);

	int AppendRightAligned(char* dst, int value);
}
}
}