#pragma once
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <windows.h>

namespace SwapChain {
	void setup(ID3D12Device* device, IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd);
	IDXGISwapChain4* getSwapChain();
	ID3D12Resource* getRtResource(UINT index);
	ID3D12DescriptorHeap* getRtvDescHeap();
	UINT getRtvDescSize();
}
