#pragma once
#include <d3d12.h>
#include <windows.h>

namespace MiniEngineIf {
	void init(ID3D12Device* device, HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight);
	void end();
	void loadModel();
	void draw(bool renderToOffscreenBuffer = true);
}
