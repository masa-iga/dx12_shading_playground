#include <windows.h>
#include "dds_loader_if.h"
#include "device_d3d12.h"
#include "render_d3d12.h"
#include "swapchain_d3d12.h"
#include "winmgr_win.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WinMgr::setup(hInstance, nCmdShow);
	DeviceD3D12::setup(WinMgr::getHwnd());
	Render::setup(DeviceD3D12::getDevice());
	SwapChain::setup(DeviceD3D12::getDevice(), DeviceD3D12::getFactory(), Render::getCommandQueue(), WinMgr::getHwnd());
	DdsLoaderIf::setDevice(DeviceD3D12::getDevice());

	Render::loadAssets(DeviceD3D12::getDevice());

	while (true)
	{
		const bool bBreak = WinMgr::handleMessage();

		if (bBreak)
			break;
		
		Render::onUpdate();
		Render::onRender();
	}

	Render::onDestroy();
	WinMgr::teardown();

	return 0;
}