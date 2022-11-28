#include "device_d3d12.h"
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "debug_win.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "DXGI.lib")

using namespace Microsoft::WRL;

[[maybe_unused]] static HRESULT enableDebugLayer();

namespace DeviceD3D12 {
	HRESULT setup()
	{
#ifdef _DEBUG
		{
			enableDebugLayer();
		}
#endif // _DEBUG

		{
			ComPtr<IDXGIFactory4> factory;
			Dbg::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

			// TODO: how to move inline ?

			// TODO: imple
		}

		return S_OK;
	}
}

HRESULT enableDebugLayer()
{
	ComPtr<ID3D12Debug> debugController = nullptr;

	auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	Dbg::assert(SUCCEEDED(hr));

	if (FAILED(hr))
		return E_FAIL;

	debugController->EnableDebugLayer();

	return S_OK;
}