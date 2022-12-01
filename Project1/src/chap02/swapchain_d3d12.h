#pragma once
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <windows.h>

namespace SwapChain {
	void createSwapChain(IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd);
}
