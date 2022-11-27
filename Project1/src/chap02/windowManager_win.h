#pragma once
#include <windows.h>

namespace WinMgr {
	HRESULT setup(HINSTANCE hInstance, int nCmdShow);
	HRESULT main();
	HRESULT teardown();
}
