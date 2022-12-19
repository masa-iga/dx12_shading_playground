#pragma once
#include <Windows.h>
#include "../../import/DirectXTex/DDSTextureLoader/DDSTextureLoader12.h"
#include "../../import/DirectXTex/DirectXTex/DDS.h"

namespace DdsLoaderIf {
    void setDevice(ID3D12Device* d3dDevice);

	HRESULT LoadDDSTextureFromFile(
        const wchar_t* szFileName,
        ID3D12Resource** texture,
        std::unique_ptr<uint8_t[]>& ddsData,
        std::vector<D3D12_SUBRESOURCE_DATA>& subresources,
        size_t maxsize = 0,
        DirectX::DDS_ALPHA_MODE* alphaMode = nullptr,
        bool* isCubeMap = nullptr);
}
