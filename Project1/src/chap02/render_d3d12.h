#pragma once
#include <d3d12sdklayers.h>

namespace Render {
	void setup(ID3D12Device* device);
	void loadAsserts(ID3D12Device* device);
}
