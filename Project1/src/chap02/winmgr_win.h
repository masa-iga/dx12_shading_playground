#pragma once
#include <windows.h>

namespace WinMgr {
	HRESULT setup(HINSTANCE hInstance, int nCmdShow);
	bool handleMessage();
	HRESULT teardown();
	HWND getHwndMain();
	HWND getHwndMiniEngine();
}
