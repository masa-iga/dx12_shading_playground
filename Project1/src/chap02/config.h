#pragma once
#include <windows.h>
#include <d3dcompiler.h>

//namespace Config {
	constexpr static UINT kRenderTargetWidth = 3840;
	constexpr static UINT kRenderTargetHeight = 2160;
	constexpr static DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr static UINT kRenderTargetBufferCount = 2;

	constexpr static D3D_ROOT_SIGNATURE_VERSION kRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1;
	constexpr static UINT kCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	constexpr static LPCSTR kVsVersion = "vs_5_1";
	constexpr static LPCSTR kPsVersion = "ps_5_1";
//}
