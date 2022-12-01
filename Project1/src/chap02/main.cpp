#include <windows.h>
#include "device_d3d12.h"
#include "winmgr_win.h"
#include "swapchain_d3d12.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WinMgr::setup(hInstance, nCmdShow);
	DeviceD3D12::setup(WinMgr::getHwnd());
	SwapChain::createSwapChain(DeviceD3D12::getFactory(), DeviceD3D12::getCommandQueue(), WinMgr::getHwnd());

	WinMgr::main();

	WinMgr::teardown();

	return 0;
}