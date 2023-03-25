#pragma once
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <windows.h>

namespace DeviceD3D12 {
	HRESULT setup();
	IDXGIFactory4* getFactory();
	ID3D12Device* getDevice();
	UINT getDescHandleIncSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
}
