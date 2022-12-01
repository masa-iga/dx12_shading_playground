#include "swapchain_d3d12.h"
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "debug_win.h"

using namespace Microsoft::WRL;

constexpr static UINT kWidth = 3840;
constexpr static UINT kHeight = 2160;
constexpr static DXGI_FORMAT kFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr static UINT kBufferCount = 2;

static ComPtr<IDXGISwapChain> s_swapChain = nullptr;

namespace SwapChain {
	void createSwapChain(IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd)
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

		Dbg::ThrowIfFailed(factory->CreateSwapChain(
			commandQueue,
			&swapChainDesc,
			s_swapChain.GetAddressOf()
		));
	}
}