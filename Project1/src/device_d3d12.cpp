#include "device_d3d12.h"
#include <wrl/client.h>
#include "debug_win.h"
#include <dxgidebug.h>

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "DXGI.lib")

using namespace Microsoft::WRL;

constexpr static D3D_FEATURE_LEVEL kD3dFeatureLevel = D3D_FEATURE_LEVEL_12_2;

[[maybe_unused]] static HRESULT enableDebugLayer();
static void createDevice();
static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter** ppAdapter);

static ComPtr<IDXGIFactory4> s_factory = nullptr;
static ComPtr<ID3D12Device> s_device = nullptr;

namespace DeviceD3D12 {
	HRESULT setup()
	{
#ifdef _DEBUG
		enableDebugLayer();
#endif // _DEBUG

		createDevice();

		return S_OK;
	}

	void tearDown()
	{
		;
	}

	void reportLiveObjects([[maybe_unused]] ReportFlag flag)
	{
#ifdef _DEBUG
		ComPtr<IDXGIDebug1> debug = nullptr;
		auto hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.ReleaseAndGetAddressOf()));
		Dbg::assert_(SUCCEEDED(hr));

		DXGI_DEBUG_RLO_FLAGS flags = static_cast<DXGI_DEBUG_RLO_FLAGS>(0);

		switch (flag) {
		case ReportFlag::kSummary: flags = DXGI_DEBUG_RLO_SUMMARY; break;
		case ReportFlag::kDetail: flags = DXGI_DEBUG_RLO_DETAIL; break;
		case ReportFlag::kIgnoreInternal: flags = DXGI_DEBUG_RLO_IGNORE_INTERNAL; break;
		case ReportFlag::kAll: flags = DXGI_DEBUG_RLO_ALL; break;
		}

		debug->ReportLiveObjects(DXGI_DEBUG_ALL, flags);
#endif // _DEBUG
	}

	IDXGIFactory4* getFactory()
	{
		return s_factory.Get();
	}

	ID3D12Device* getDevice()
	{
		return s_device.Get();
	}

	UINT getDescHandleIncSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
	{
		Dbg::assert_(s_device != nullptr);
		return s_device->GetDescriptorHandleIncrementSize(heapType);
	}
}

HRESULT enableDebugLayer()
{
	ComPtr<ID3D12Debug> debugController = nullptr;

	auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	Dbg::assert_(SUCCEEDED(hr));

	if (FAILED(hr))
		return E_FAIL;

	debugController->EnableDebugLayer();

	return S_OK;
}

void createDevice()
{
	Dbg::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&s_factory)));

	ComPtr<IDXGIAdapter> hardwareAdapter = { };
	GetHardwareAdapter(s_factory.Get(), hardwareAdapter.ReleaseAndGetAddressOf());

	Dbg::ThrowIfFailed(D3D12CreateDevice(
		hardwareAdapter.Get(),
		kD3dFeatureLevel,
		IID_PPV_ARGS(&s_device)
	));
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

