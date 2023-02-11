#include "swapchain_d3d12.h"
#include "config.h"
#include <d3d12sdklayers.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include "debug_win.h"

using namespace Microsoft::WRL;

namespace {
	ComPtr<IDXGISwapChain4> s_swapChain = nullptr;
	ComPtr<ID3D12DescriptorHeap> s_rtvHeap = nullptr;
	ComPtr<ID3D12Resource> s_renderTargets[Config::kRenderTargetBufferCount];
	ComPtr<ID3D12Resource> s_depthResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> s_depthDescHeap = nullptr;
	UINT s_rtvDescriptorSize = 0;

	void createSwapChain(IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd);
	void createFrameBufferDescHeap(ID3D12Device* device);
	void createFrameBufferResource(ID3D12Device* device);
	void createDepthResource(ID3D12Device* device);
	void createDepthDescHeap(ID3D12Device* device);
}

namespace SwapChain {
	void setup(ID3D12Device* device, IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd)
	{
		createSwapChain(factory, commandQueue, hwnd);
		createFrameBufferDescHeap(device);
		createFrameBufferResource(device);
		
		createDepthResource(device);
		createDepthDescHeap(device);
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

	ID3D12DescriptorHeap* getDepthDescHeap()
	{
		return s_depthDescHeap.Get();
	}
}

namespace {
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

	void createFrameBufferDescHeap(ID3D12Device* device)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC desc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = 2,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0,
		};

		Dbg::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(s_rtvHeap.ReleaseAndGetAddressOf())));
		Dbg::ThrowIfFailed(s_rtvHeap->SetName(L"DescHeap_FrameBuffer"));

		s_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	void createFrameBufferResource(ID3D12Device* device)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(s_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (int32_t i = 0; i < Config::kRenderTargetBufferCount; ++i)
		{
			Dbg::ThrowIfFailed(s_swapChain->GetBuffer(i, IID_PPV_ARGS(s_renderTargets[i].ReleaseAndGetAddressOf())));
			device->CreateRenderTargetView(s_renderTargets[i].Get(), nullptr, handle);
			Dbg::ThrowIfFailed(s_renderTargets[i]->SetName(L"FrameBuffer"));
			handle.Offset(1, s_rtvDescriptorSize);
		}
	}

	void createDepthResource(ID3D12Device* device)
	{
		const CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Config::kRenderTargetWidth, Config::kRenderTargetHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		const D3D12_CLEAR_VALUE clearVal = {
			.Format = DXGI_FORMAT_D32_FLOAT,
			.DepthStencil = {
				.Depth = 1.0f,
				.Stencil = 0,
			},
		};

		Dbg::ThrowIfFailed(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearVal,
			IID_PPV_ARGS(s_depthResource.ReleaseAndGetAddressOf())));
		Dbg::ThrowIfFailed(s_depthResource->SetName(L"DepthRenderTarget"));
	}

	void createDepthDescHeap(ID3D12Device* device)
	{
		{
			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0,
			};

			Dbg::ThrowIfFailed(device->CreateDescriptorHeap(
				&heapDesc,
				IID_PPV_ARGS(s_depthDescHeap.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_depthDescHeap->SetName(L"DescHeap_depthRenderTarget"));
		}
		auto handle = s_depthDescHeap->GetCPUDescriptorHandleForHeapStart();

		const D3D12_DEPTH_STENCIL_VIEW_DESC desc = {
			.Format = DXGI_FORMAT_D32_FLOAT,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags = D3D12_DSV_FLAG_NONE,
			.Texture2D = 0,
		};
		device->CreateDepthStencilView(s_depthResource.Get(), &desc, handle);
	}
}
