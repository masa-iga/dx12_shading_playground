#pragma once
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <windows.h>

namespace Render {
	void setup(ID3D12Device* device, HWND hwnd);
	void loadAssets(ID3D12Device* device);
	void onUpdate();
	void onRender();
	void onDestroy();
	ID3D12CommandQueue* getCommandQueue();
}
