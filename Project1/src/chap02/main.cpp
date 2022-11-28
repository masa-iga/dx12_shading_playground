#include <windows.h>
#include "device_d3d12.h"
#include "windowManager_win.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WinMgr::setup(hInstance, nCmdShow);
	DeviceD3D12::setup();

	WinMgr::main();

	WinMgr::teardown();

	return 0;
}