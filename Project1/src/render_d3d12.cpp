#include "render_d3d12.h"
#include <d3dx12.h>
#include <wrl/client.h>
#include "config.h"
#include "debug_win.h"
#include "imgui_if.h"
#include "miniEngine_if.h"
#include "model_simple_triangle.h"
#include "swapchain_d3d12.h"
#include "toolkit.h"
#include "timestamp.h"

#pragma comment(lib, "D3DCompiler.lib")

using namespace Microsoft::WRL;

namespace {
	SimpleTriangleModel s_simpleTriangleModel;

	ComPtr<ID3D12CommandQueue> s_commandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> s_commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> s_commandList = nullptr;
	ComPtr<ID3D12Fence> s_fence = nullptr;
	UINT64 s_fenceValue = 0;
	HANDLE s_fenceEvent = nullptr;
	UINT s_frameIndex = 0;
	Timestamp s_timestamp;
	float s_gpuTimeInUsec = 0.0f;

	void createCommandAllocator(ID3D12Device* device);
	void createCommandQueue(ID3D12Device* device);
	void createCommandList(ID3D12Device* device);
	void createFence(ID3D12Device* device);
	void waitForPreviousFrame();
	void startFrame();
	void endFrame();
	void populateCommandList();
}

namespace Render {
	void setup(ID3D12Device* device, HWND hwndForImgui, HWND hwndForMiniEngine)
	{
		createCommandAllocator(device);
		createCommandQueue(device);

		MiniEngineIf::init(device, hwndForMiniEngine, Config::kRenderTargetWidth, Config::kRenderTargetHeight);
		Toolkit::init(device);
		ImguiIf::init(device, hwndForImgui);
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "GPU time [us]", std::vector<const float*>{ &s_gpuTimeInUsec }, ImguiIf::ParamType::kGpuTime);
		s_timestamp.init(device);
		s_timestamp.setGpuFreq(s_commandQueue.Get());
	}

	void loadAssets(ID3D12Device* device)
	{
		s_frameIndex = SwapChain::getSwapChain()->GetCurrentBackBufferIndex();
		createCommandList(device);
		createFence(device);
		waitForPreviousFrame();

		// create resource for each model
		{
			s_simpleTriangleModel.createResource(device);
		}

		// upload textures for each model
		Dbg::ThrowIfFailed(s_commandAllocator->Reset());
		Dbg::ThrowIfFailed(s_commandList->Reset(s_commandAllocator.Get(), nullptr));
		{
			s_simpleTriangleModel.uploadTextures(device, s_commandList.Get());
		}
		Dbg::ThrowIfFailed(s_commandList->Close());

		ID3D12CommandList* ppCommandLists[] = { s_commandList.Get() };
		s_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		waitForPreviousFrame();

		// release temporary buffers
		{
			s_simpleTriangleModel.releaseTemporaryBuffers();
		}

		MiniEngineIf::loadModel();
	}

	void onUpdate()
	{
		MiniEngineIf::handleInput();
	}

	void onRender()
	{
		startFrame();

		MiniEngineIf::beginFrame(&s_timestamp);
		MiniEngineIf::draw(true);
		MiniEngineIf::endFrame(&s_timestamp);

		populateCommandList();

		endFrame();

		ID3D12CommandList* ppCommandLists[] = { s_commandList.Get() };
		s_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		{
			const UINT syncInterval = 1;
			const UINT flags = 0;
			Dbg::ThrowIfFailed(SwapChain::getSwapChain()->Present(syncInterval, flags));

			waitForPreviousFrame();
		}
	}

	void onDestroy()
	{
		waitForPreviousFrame();
		::CloseHandle(s_fenceEvent);
		MiniEngineIf::end();
		ImguiIf::shutdown();
	}

	ID3D12CommandQueue* getCommandQueue()
	{
		return s_commandQueue.Get();
	}
}

namespace {
	void createCommandAllocator(ID3D12Device* device)
	{
		Dbg::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(s_commandAllocator.ReleaseAndGetAddressOf())));
		Dbg::ThrowIfFailed(s_commandAllocator->SetName(L"Render_CommandAllocator"));
	}

	void createCommandQueue(ID3D12Device* device)
	{
		const D3D12_COMMAND_QUEUE_DESC queueDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0,
		};

		Dbg::ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(s_commandQueue.ReleaseAndGetAddressOf())));
		Dbg::ThrowIfFailed(s_commandQueue->SetName(L"Render_CommandQueue"));
	}

	void createCommandList(ID3D12Device* device)
	{
		Dbg::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, s_commandAllocator.Get(), nullptr, IID_PPV_ARGS(s_commandList.ReleaseAndGetAddressOf())));
		Dbg::ThrowIfFailed(s_commandList->SetName(L"Render_CommandList"));

		Dbg::ThrowIfFailed(s_commandList.Get()->Close());
	}

	void createFence(ID3D12Device* device)
	{
		constexpr UINT64 initVal = 0;
		Dbg::ThrowIfFailed(device->CreateFence(initVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(s_fence.ReleaseAndGetAddressOf())));
		s_fenceValue = 1;

		s_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (s_fenceEvent == nullptr)
		{
			Dbg::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

	void waitForPreviousFrame()
	{
		const UINT64 fence = s_fenceValue;
		Dbg::ThrowIfFailed(s_commandQueue->Signal(s_fence.Get(), fence));
		s_fenceValue++;

		if (s_fence->GetCompletedValue() < fence)
		{
			Dbg::ThrowIfFailed(s_fence->SetEventOnCompletion(fence, s_fenceEvent));
			Dbg::assert_(WaitForSingleObject(s_fenceEvent, INFINITE) == WAIT_OBJECT_0);
		}

		s_frameIndex = SwapChain::getSwapChain()->GetCurrentBackBufferIndex();
	}

	void startFrame()
	{
		Dbg::ThrowIfFailed(s_commandAllocator->Reset());
		Dbg::ThrowIfFailed(s_commandList->Reset(s_commandAllocator.Get(), nullptr));
		ImguiIf::startFrame();
		s_gpuTimeInUsec = static_cast<float>(s_timestamp.computeDiffInUsec(Timestamp::Point::kFrameBegin, Timestamp::Point::kFrameEnd));
		s_timestamp.flip();
	}

	void endFrame()
	{
		Dbg::ThrowIfFailed(s_commandList->Close());
	}

	void populateCommandList()
	{
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(SwapChain::getRtResource(s_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			s_commandList->ResourceBarrier(1, &barrier);
		}

		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(SwapChain::getRtvDescHeap()->GetCPUDescriptorHandleForHeapStart(), s_frameIndex, SwapChain::getRtvDescSize());

		// clear color
		{
			const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
			s_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}

		// render model(s)
		{
			const CD3DX12_VIEWPORT vp(0.0f, 0.0f, static_cast<float>(Config::kRenderTargetWidth), static_cast<float>(Config::kRenderTargetHeight));
			const CD3DX12_RECT scrt(0, 0, Config::kRenderTargetWidth, Config::kRenderTargetHeight);
			s_commandList->RSSetViewports(1, &vp);
			s_commandList->RSSetScissorRects(1, &scrt);
			s_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

			s_simpleTriangleModel.draw(s_commandList.Get());
		}

		// render texture of MiniEngine to frame buffer
		{
			{
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					MiniEngineIf::getRenderTargetResource(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				s_commandList->ResourceBarrier(1, &barrier);
			}

			Toolkit::copyTextureToTarget(s_commandList.Get(), MiniEngineIf::getRenderTargetResource());
		}

		// UI
		{
			ImguiIf::update();
			ImguiIf::render(s_commandList.Get());
		}

		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(SwapChain::getRtResource(s_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			s_commandList->ResourceBarrier(1, &barrier);
		}

		// clear render targets passed to MiniEngine
		{
			{
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					MiniEngineIf::getRenderTargetResource(),
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_RENDER_TARGET);
				s_commandList->ResourceBarrier(1, &barrier);
			}

			MiniEngineIf::clearRenderTarget(s_commandList.Get());
			MiniEngineIf::clearDepthRenderTarget(s_commandList.Get());
		}
	}
}
