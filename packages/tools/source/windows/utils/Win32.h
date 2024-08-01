#pragma once

#include <iostream>
#include <Windows.h>
#include <optional>
#include <map>
#include <vector>
#include <set>
#include <mutex>
#include <functional>

namespace l {
namespace win32 {

	using WindowCallback = std::function<bool(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)>;
	
	LRESULT __stdcall WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	class WindowData {
	public:
		WindowData() = delete;
		WindowData(std::wstring wndName, WNDCLASSEXW wndClass, HWND hWnd);

		std::wstring mWndName;
		WNDCLASSEXW mWndClass;
		HWND mHWnd;
		WindowCallback mWindowHandler;
	};

	std::optional<WindowData> createWindow(std::wstring name, uint32_t sizex, uint32_t sizey, int showWindowCommand, int windowStyle, WindowCallback cb = nullptr);

	bool processMessages();
}
}