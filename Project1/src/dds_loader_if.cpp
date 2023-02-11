#include "dds_loader_if.h"
#include "../../import/DirectXTex/DDSTextureLoader/DDSTextureLoader12.h"
#include <wrl/client.h>
#include "debug_win.h"

using namespace Microsoft::WRL;

static ComPtr<ID3D12Device> s_device = nullptr;

void DdsLoaderIf::setDevice(ID3D12Device* d3dDevice)
{
    s_device = d3dDevice;
}

HRESULT DdsLoaderIf::LoadDDSTextureFromFile(
        const wchar_t* szFileName,
        ID3D12Resource** texture,
        std::unique_ptr<uint8_t[]>& ddsData,
        std::vector<D3D12_SUBRESOURCE_DATA>& subresources,
        size_t maxsize,
        DirectX::DDS_ALPHA_MODE* alphaMode,
        bool* isCubeMap)
{
    Dbg::assert_(s_device != nullptr);
    return DirectX::LoadDDSTextureFromFile(s_device.Get(), szFileName, texture, ddsData, subresources, maxsize, alphaMode, isCubeMap);
}
