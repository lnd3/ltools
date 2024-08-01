#include "Win32.h"
#include "logging/Log.h"

#include <iostream>
#include <Windows.h>
#include <optional>
#include <map>
#include <vector>
#include <set>
#include <gl/GL.h>

namespace {
	std::unordered_map<HWND, l::win32::WindowCallback> gCBMapping1;
	std::unordered_map<std::wstring_view, l::win32::WindowCallback> gCBMapping2;
	std::mutex gCBMappingMutex;

	std::atomic_int32_t WindowId = 0;

	std::wstring GetClassName(HWND hwnd, size_t maxCount = 20) {
		WCHAR classNameBuffer[128];
		auto len = GetClassNameW(hwnd, classNameBuffer, static_cast<int>(maxCount));
		if (len > 0) {
			return std::wstring(classNameBuffer, static_cast<size_t>(len));
		}
		return {};
	}

	bool InvokeWindowCallback(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		bool msgConsumed = false;

		l::win32::WindowCallback callback = nullptr;

		{
			std::lock_guard<std::mutex> lock(gCBMappingMutex);
			auto it = gCBMapping1.find(hwnd);
			if (it != gCBMapping1.end()) {
				if (it->second) {
					callback = it->second;
				}
			}
			else {
				auto className = GetClassName(hwnd);
				auto it2 = gCBMapping2.find(className);
				if (it2 != gCBMapping2.end()) {
					if (it2->second) {
						callback = it2->second;
					}
				}
				else {
					LOG(LogError) << "Failed to locate window callback: " << l::string::narrow(className);
				}
			}
		}

		if (callback) {
			msgConsumed = callback(hwnd, msg, wp, lp);
		}

		return msgConsumed;
	}

	void EraseWindowCallback(HWND hwnd) {
		std::lock_guard<std::mutex> lock(gCBMappingMutex);
		gCBMapping1.erase(hwnd);

		auto className = GetClassName(hwnd);
		gCBMapping2.erase(className);
	}
}

namespace l {
	namespace win32 {

		LRESULT __stdcall WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			bool msgConsumed = InvokeWindowCallback(hwnd, msg, wp, lp);

			if (msgConsumed) {
				return 0L;
			}

			// Default behaviour
			switch (msg)
			{
			case WM_ACTIVATEAPP:
				LOG(LogDebug) << "[" << hwnd << "] WM_ACTIVATEAPP recieved [" << wp << ", " << lp << "]";
				break;
			case WM_ACTIVATE:
				LOG(LogDebug) << "[" << hwnd << "] WM_ACTIVATE recieved [" << wp << ", " << lp << "]";
				break;
			case WM_NCCREATE:
				LOG(LogDebug) << "[" << hwnd << "] WM_NCCREATE recieved [" << wp << ", " << lp << "]";
				break;
			case WM_GETMINMAXINFO:
				//LOG(LogDebug) << "[" << hwnd << "] WM_GETMINMAXINFO recieved [" << wp << ", " << lp << "]";
				break;
			case WM_CREATE:
				LOG(LogDebug) << "[" << hwnd << "] WM_CREATE recieved [" << wp << ", " << lp << "]";
				break;
			case WM_GETICON: // Called regulary to update large and small app icon
				break;
			case WM_CLOSE:
				LOG(LogDebug) << "[" << hwnd << "] WM_CLOSE recieved [" << wp << ", " << lp << "]";
				break;
			case WM_DESTROY:
				LOG(LogDebug) << "[" << hwnd << "] WM_DESTROY recieved [" << wp << ", " << lp << "]";
				break;
			case WM_NCDESTROY:
				LOG(LogDebug) << "[" << hwnd << "] WM_NCDESTROY recieved [" << wp << ", " << lp << "]";
				EraseWindowCallback(hwnd);
			}
			return DefWindowProcW(hwnd, msg, wp, lp);
		}

		WindowData::WindowData(std::wstring wndName, WNDCLASSEXW wndClass, HWND hWnd) : mWndName(wndName), mWndClass(wndClass), mHWnd(hWnd) {
		}

		std::optional<WindowData> createWindow(std::wstring name, uint32_t sizex, uint32_t sizey, int showWindowCommand, int windowStyle, WindowCallback cb)
		{
			std::wstring className = L"w" + std::to_wstring(WindowId++);
			WNDCLASSEXW wndClassEx = { sizeof(WNDCLASSEXW), CS_DBLCLKS, WindowProcedure, 0, 0, GetModuleHandle(0), LoadIcon(0,IDI_APPLICATION), LoadCursor(0,IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), 0, className.c_str(), LoadIcon(0,IDI_APPLICATION) };

			auto result = RegisterClassExW(&wndClassEx);
			if (result)
			{
				{
					std::lock_guard<std::mutex> lock(gCBMappingMutex);
					gCBMapping2.insert_or_assign(className, cb);
				}

				HWND hWnd = CreateWindowExW(0, className.c_str(), name.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, sizex, sizey, 0, 0, GetModuleHandle(0), 0);

				{
					std::lock_guard<std::mutex> lock(gCBMappingMutex);
					gCBMapping1.insert_or_assign(hWnd, cb);
				}

				if (hWnd)
				{
					ShowWindow(hWnd, showWindowCommand);
					return std::make_optional<WindowData>(name, std::move(wndClassEx), hWnd);
				}
				else {
					LOG(LogError) << "Failed to create window, error " << GetLastError();
				}
			}
			else {
				LOG(LogError) << "Failed to register window class. Error: " << GetLastError();
			}
			LOG(LogError) << "See https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes for more info";
			return std::nullopt;
		}

		bool processMessages() {
			MSG msg;
			if (PeekMessage(&msg, 0, 0, 0, 0)) {
				if (GetMessage(&msg, 0, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					return false;
				}
			}
			std::lock_guard lock(gCBMappingMutex);
			if (gCBMapping1.empty()) {
				return false;
			}
			return true;
		}
	}
}