#pragma once
#include <d3d12.h>
#include <Windows.h>

namespace Toolkit {
	void init(ID3D12Device* device);
	void copyTextureToTarget(ID3D12GraphicsCommandList* list);
}
