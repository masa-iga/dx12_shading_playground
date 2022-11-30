#include "device_d3d12.h"
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "debug_win.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "DXGI.lib")

using namespace Microsoft::WRL;

constexpr static D3D_FEATURE_LEVEL kD3dFeatureLevel = D3D_FEATURE_LEVEL_12_2;
constexpr static UINT kWidth = 3840;
constexpr static UINT kHeight = 2160;
constexpr static DXGI_FORMAT kFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr static UINT kBufferCount = 2;

[[maybe_unused]] static HRESULT enableDebugLayer();
static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter** ppAdapter);
static void createDevice();
static void createCommandQueue();
static void createSwapChain(HWND hwnd);

static ComPtr<IDXGIFactory4> s_factory = nullptr;
static ComPtr<ID3D12Device> s_device = nullptr;
static ComPtr<ID3D12CommandQueue> s_commandQueue = nullptr;
static ComPtr<IDXGISwapChain> s_swapChain = nullptr;

namespace DeviceD3D12 {
	HRESULT setup(HWND hwnd)
	{
#ifdef _DEBUG
		enableDebugLayer();
#endif // _DEBUG

		createDevice();
		createCommandQueue();
		createSwapChain(hwnd);

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

void createSwapChain(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {
	.BufferDesc = {
		.Width = kWidth,
		.Height = kHeight,
		.RefreshRate = {
			.Numerator = 0,
			.Denominator = 0,
		},
		.Format = kFormat,
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
	},
	.SampleDesc = {
		.Count = 1,
		.Quality = 0,
	},
	.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
	.BufferCount = kBufferCount,
	.OutputWindow = hwnd,
	.Windowed = TRUE,
	.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
	.Flags = 0,
	};

	Dbg::ThrowIfFailed(s_factory->CreateSwapChain(
		s_commandQueue.Get(),
		&swapChainDesc,
		s_swapChain.GetAddressOf()
	));
}
