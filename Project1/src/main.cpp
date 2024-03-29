#include <windows.h>
#include "dds_loader_if.h"
#include "device_d3d12.h"
#include "imgui_if.h"
#include "fps.h"
#include "render_d3d12.h"
#include "swapchain_d3d12.h"
#include "winmgr_win.h"

static uint64_t s_frame = 0;

void setup(HINSTANCE hInstance, int nCmdShow);
void load();
void teardown();
void computeFps();

int WINAPI wWinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] PWSTR pCmdLine, int nCmdShow)
{
	setup(hInstance, nCmdShow);

	load();

	while (true)
	{
		const bool bBreak = WinMgr::handleMessage();

		if (bBreak)
			break;
		
		Render::onUpdate();
		Render::onRender();
		computeFps();
		++s_frame;
	}

	teardown();

	return 0;
}

void setup(HINSTANCE hInstance, int nCmdShow)
{
	WinMgr::setup(hInstance, nCmdShow);
	DeviceD3D12::setup();
	Render::setup(DeviceD3D12::getDevice(), WinMgr::getHwnd(WinMgr::Handle::kMain), WinMgr::getHwnd(WinMgr::Handle::kMiniEngine));
	SwapChain::setup(DeviceD3D12::getDevice(), DeviceD3D12::getFactory(), Render::getCommandQueue(), WinMgr::getHwnd(WinMgr::Handle::kMain));
	DdsLoaderIf::setDevice(DeviceD3D12::getDevice());

	ImguiIf::printParams<uint64_t>(ImguiIf::VarType::kUint64, "Frame", std::vector<const uint64_t*>{ &s_frame }, ImguiIf::ParamType::kFrame);
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Fps", std::vector<const float*>{ Fps::getFpsRef() }, ImguiIf::ParamType::kGeneral);
}

void load()
{
	Render::loadAssets(DeviceD3D12::getDevice());
}

void teardown()
{
	Render::onDestroy();
	DeviceD3D12::tearDown();
	WinMgr::teardown();
}

void computeFps()
{
	Fps::compute();
}
