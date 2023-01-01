#pragma once
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <windows.h>

namespace Render {
	void setup(HWND hwnd, ID3D12Device* device);
	void loadAssets(ID3D12Device* device);
	void onUpdate();
	void onRender();
	void onDestroy();
	ID3D12CommandQueue* getCommandQueue();
}
