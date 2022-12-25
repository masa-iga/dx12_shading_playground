#pragma once
#include <Windows.h>
#include <algorithm>
#include <cstdint>
#include <d3d12.h>

namespace Util {
#if defined(_WIN32) || defined(_WIN64)
	template <class T>
	const UINT getAlignedContantBufferSize(T size)
	{
		constexpr size_t kAlign = 256;
		return static_cast<UINT>((std::min)(static_cast<T>(kAlign), static_cast<T>(size + kAlign - 1)));
	}

	void debugPrint(ID3D12Resource* resource);
#endif // #if defined(_WIN32) || defined(_WIN64)
}
