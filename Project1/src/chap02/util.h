#pragma once
#include <algorithm>
#include <cstdint>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#endif // #if defined(_WIN32) || defined(_WIN64)

namespace Util {
	template <class T>
	const UINT getAlignedContantBufferSize(T size)
	{
		constexpr size_t kAlign = 256;
		return static_cast<UINT>((std::min)(static_cast<T>(kAlign), static_cast<T>(size + kAlign - 1)));
	}

#if defined(_WIN32) || defined(_WIN64)
	HRESULT compileVsShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCWSTR shaderFile, LPCSTR entrypoint);
	HRESULT compilePsShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCWSTR shaderFile, LPCSTR entrypoint);
	void debugPrint(ID3D12Resource* resource);
#endif // #if defined(_WIN32) || defined(_WIN64)
}
