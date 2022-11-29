#include "device_d3d12.h"
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "debug_win.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "DXGI.lib")

using namespace Microsoft::WRL;

[[maybe_unused]] static HRESULT enableDebugLayer();
static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter** ppAdapter);

constexpr D3D_FEATURE_LEVEL kD3dFeatureLevel = D3D_FEATURE_LEVEL_12_2;

ComPtr<ID3D12Device> s_device;

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

			ComPtr<IDXGIAdapter> hardwareAdapter = { };
			GetHardwareAdapter(factory.Get(), hardwareAdapter.ReleaseAndGetAddressOf());

			Dbg::ThrowIfFailed(D3D12CreateDevice(
				hardwareAdapter.Get(),
				kD3dFeatureLevel,
				IID_PPV_ARGS(&s_device)
			));
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

void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter** ppAdapter)
{
	*ppAdapter = nullptr;
	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		IDXGIAdapter1* pAdapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
		{
			// No more adapters to enumerate.
			break;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(pAdapter, kD3dFeatureLevel, _uuidof(ID3D12Device), nullptr)))
		{
			*ppAdapter = pAdapter;

			{
				DXGI_ADAPTER_DESC1 pDesc = { };
				Dbg::ThrowIfFailed(SUCCEEDED(pAdapter->GetDesc1(&pDesc)));
				Dbg::print("Adapter%d: %ls\n", adapterIndex, pDesc.Description);
			}
			return;
		}
		pAdapter->Release();
	}
}