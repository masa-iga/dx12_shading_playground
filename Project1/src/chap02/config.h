#pragma once
#include <windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>

namespace Config {
	constexpr static UINT kRenderTargetWidth = 1920;
	constexpr static UINT kRenderTargetHeight = 1080;
	constexpr static DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr static UINT kRenderTargetBufferCount = 2;

	constexpr static D3D_ROOT_SIGNATURE_VERSION kRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1;
	const static UINT kCompileFlags1 = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	const static UINT kCompileFlags2 = 0;
	const static LPCSTR kVsVersion = "vs_5_1";
	const static LPCSTR kPsVersion = "ps_5_1";
}
