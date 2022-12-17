#include "swapchain_d3d12.h"
#include "config.h"
#include <d3d12sdklayers.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include "debug_win.h"

using namespace Microsoft::WRL;

static ComPtr<IDXGISwapChain4> s_swapChain = nullptr;
static ComPtr<ID3D12DescriptorHeap> s_rtvHeap = nullptr;
static ComPtr<ID3D12Resource> s_renderTargets[Config::kRenderTargetBufferCount];
static UINT s_rtvDescriptorSize = 0;

static void createSwapChain(IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd);
static void createDescHeap(ID3D12Device* device);
static void createResource(ID3D12Device* device);

namespace SwapChain {
	void setup(ID3D12Device* device, IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd)
	{
		createSwapChain(factory, commandQueue, hwnd);
		createDescHeap(device);
		createResource(device);
	}

	IDXGISwapChain4* getSwapChain()
	{
		return s_swapChain.Get();
	}

	ID3D12Resource* getRtResource(UINT index)
	{
		return s_renderTargets[index].Get();
	}

	ID3D12DescriptorHeap* getRtvDescHeap()
	{
		return s_rtvHeap.Get();
	}

	UINT getRtvDescSize()
	{
		return s_rtvDescriptorSize;
	}
}

void createSwapChain(IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {
	.BufferDesc = {
		.Width = Config::kRenderTargetWidth,
		.Height = Config::kRenderTargetHeight,
		.RefreshRate = {
			.Numerator = 0,
			.Denominator = 0,
		},
		.Format = Config::kRenderTargetFormat,
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
	},
	.SampleDesc = {
		.Count = 1,
		.Quality = 0,
	},
	.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
	.BufferCount = Config::kRenderTargetBufferCount,
	.OutputWindow = hwnd,
	.Windowed = TRUE,
	.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
	.Flags = 0,
	};

	Dbg::ThrowIfFailed(factory->CreateSwapChain(
		commandQueue,
		&swapChainDesc,
		reinterpret_cast<IDXGISwapChain**>(s_swapChain.GetAddressOf())
	));
}

void createDescHeap(ID3D12Device* device)
{
	const D3D12_DESCRIPTOR_HEAP_DESC desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = 2,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0,
	};

	Dbg::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(s_rtvHeap.ReleaseAndGetAddressOf())));

	s_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void createResource(ID3D12Device* device)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(s_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int32_t i = 0; i < Config::kRenderTargetBufferCount; ++i)
	{
		Dbg::ThrowIfFailed(s_swapChain->GetBuffer(i, IID_PPV_ARGS(s_renderTargets[i].ReleaseAndGetAddressOf())));
		device->CreateRenderTargetView(s_renderTargets[i].Get(), nullptr, handle);
		handle.Offset(1, s_rtvDescriptorSize);
	}
}
