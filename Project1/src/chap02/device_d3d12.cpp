#include "device_d3d12.h"
#include <wrl/client.h>
#include "debug_win.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "DXGI.lib")

using namespace Microsoft::WRL;

constexpr static D3D_FEATURE_LEVEL kD3dFeatureLevel = D3D_FEATURE_LEVEL_12_2;

[[maybe_unused]] static HRESULT enableDebugLayer();
static void createDevice();
static void createCommandQueue();
static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter** ppAdapter);

static ComPtr<IDXGIFactory4> s_factory = nullptr;
static ComPtr<ID3D12Device> s_device = nullptr;
static ComPtr<ID3D12CommandQueue> s_commandQueue = nullptr;

namespace DeviceD3D12 {
	HRESULT setup(HWND hwnd)
	{
#ifdef _DEBUG
		enableDebugLayer();
#endif // _DEBUG

		createDevice();
		createCommandQueue();

		return S_OK;
	}

	IDXGIFactory4* getFactory()
	{
		return s_factory.Get();
	}

	ID3D12Device* getDevice()
	{
		return s_device.Get();
	}

	ID3D12CommandQueue* getCommandQueue()
	{
		return s_commandQueue.Get();
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

void createCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC queueDesc = {
	.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
	.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
	.NodeMask = 0,
	};

	Dbg::ThrowIfFailed(s_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(s_commandQueue.ReleaseAndGetAddressOf())));
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

