#pragma once
#include <windows.h>

namespace WinMgr {
	enum class Handle {
		kMain,
		kMiniEngine,
	};

	HRESULT setup(HINSTANCE hInstance, int nCmdShow);
	bool handleMessage();
	HRESULT teardown();
	HWND getHwndMain();
	HWND getHwndMiniEngine();
	bool isWindowActive(Handle handle);
}
