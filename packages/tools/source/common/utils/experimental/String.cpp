#include "tools/utils/experimental/String.h"

#include "logging/Log.h"

namespace l {
namespace x {

string::SString string::CreateString(char* src, int max_len)
{
	SString string;
	string.str = src;
	string.cur_len = 0;
	string.max_len = max_len;
	EndString(string);
	return string;
}

string::SString string::CreateString(const char* src)
{
	SString string;
	string.str = (char*)src;
	string.cur_len = StrLen(src);
	string.max_len = string.cur_len;
	return string;
}

void string::EndString(SString& dst)
{
	*(dst.str + dst.cur_len) = 0;
}

void string::Append(SString& dst, const char src)
{
	*(dst.str + dst.cur_len++) = src;
	EndString(dst);

	ASSERT(dst.cur_len < dst.max_len) << "SString has a illegal size (size must be less " << std::to_string(dst.max_len) << ", but is " << std::to_string(dst.cur_len);
}

void string::Append(SString& dst, const char* src)
{
	for (int i = 0; src[i] != 0; i++) {
		Append(dst, src[i]);
	}
}

void string::Append(SString& dst, SString& src)
{
	for (int i = 0; i < src.cur_len; i++) {
		Append(dst, src.str[i]);
	}
}

void string::Append(SString& dst, int value)
{
	int divisor = 1000000000; // value is never larger than 10^10, so start with 10^9.
	
	// find the closest base 10 value larger then value
	for (; value < divisor && divisor > 0;) {
		divisor /= 10;
	}
	// find the quotient for every base 10 division down to zero
	for (; value > 0;) {
		int rest = value / divisor;
		value = value % divisor;
		divisor /= 10;

		Append(dst, '0' + rest);
	}
}

bool string::Equals(const SString& src0, const SString& src1)
{
	if (src0.cur_len != src1.cur_len) {
		return false;
	}
	for (int i = 0; i < src0.cur_len; i++) {
		if (src0.str[i] != src1.str[i]) {
			return false;
		}
	}
	return true;
}

bool string::Equals(const SString& src0, const char* src1)
{
	for (int i = 0; i < src0.cur_len; i++) {
		if (src0.str[i] != src1[i]) {
			return false;
		}
	}
	if (src0.cur_len != StrLen(src1)) {
		return false;
	}
	return true;
}

int string::StrLen(SString& dst)
{
	return dst.cur_len;
}

int string::StrLen(const char* src, int max_chars)
{
	int i = 0;
	while (*(src + i) != 0 && i < max_chars) {
		i++;
	}
	return i;
}

void string::StrCpy(char* dst, const char* src, int max_chars)
{
	char* _dst = (char*)dst;
	char* _src = (char*)src;

	bool dst_size_changed = false;
	for (int i = 0; i < max_chars && *_src != 0; i++) {
		if (*_dst == 0) {
			dst_size_changed = true;
		}
		*_dst++ = *_src++;
	}
	if (dst_size_changed) { // write new null terminator at end of new string
		*_dst = 0;
	}
}

void string::SetCharAt(char* dst, int at, char c)
{
	*(dst + at) = c;
}

int string::Append(char* dst, int value)
{
	char* dst_2 = dst;
	int divisor = 1000000000; // value is never larger than 10^10, so start with 10^9.

	// find the closest base 10 value larger then value
	for (; value < divisor && divisor > 0;) {
		divisor /= 10;
	}
	// find the quotient for every base 10 division down to zero
	for (; value > 0;) {
		int rest = value / divisor;
		value = value % divisor;
		divisor /= 10;

		*(dst_2++) = (char)('0' + rest);
	}

	return static_cast<int>(dst_2 - dst);
}

int string::Append(char* dst, const char* src)
{
	int dst_length = StrLen(dst);
	StrCpy(dst + dst_length, src);
	return dst_length;
}

int string::AppendRightAligned(char* dst, int value)
{
	char* dst_2 = dst;
	for (; value > 0;) {
		int rest = value % 10;
		value = value / 10;
		*(dst_2--) = (char)('0' + rest);
	}

	return static_cast<int>(dst - dst_2);
}
}
}