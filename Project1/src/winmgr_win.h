#pragma once
#include <windows.h>
#include <minwindef.h>

namespace WinMgr {
	enum class Handle {
		kMain,
		kMiniEngine,
	};

	class Iobserber
	{
	public:
		virtual void update(WPARAM wParam, LPARAM lParam) = 0;
	};

	HRESULT setup(HINSTANCE hInstance, int nCmdShow);
	bool handleMessage();
	HRESULT teardown();
	HWND getHwnd(Handle handle);
	bool isWindowActive(Handle handle);
	bool addObserver(Iobserber* obserber);
	bool removeObserver(Iobserber* obserber);
}
