#pragma once
#include <d3d12.h>
#include <Windows.h>

namespace Toolkit {
	void init();
	void renderTextureToTarget(ID3D12GraphicsCommandList* list);
}
