#pragma once
#include <d3d12.h>
#include <windows.h>

namespace MiniEngineIf {
	void init(ID3D12Device* device, HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight);
	void end();
	void loadModel();
	ID3D12Resource* getRenderTargetResource();
	void beginFrame();
	void endFrame();
	void clearRenderTarget(ID3D12GraphicsCommandList* commandList);
	void draw(bool renderToOffscreenBuffer = true);
}
