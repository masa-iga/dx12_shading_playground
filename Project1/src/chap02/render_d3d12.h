#pragma once
#include <d3d12.h>
#include <d3d12sdklayers.h>

namespace Render {
	void setup(ID3D12Device* device);
	void loadAssets(ID3D12Device* device);
	ID3D12CommandQueue* getCommandQueue();
}
