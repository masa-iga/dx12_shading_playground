#include "miniEngine_if.h"
#include <filesystem>
#include "../../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"
#include "debug_win.h"
#include "setup_models.h"
#include "timestamp.h"

#pragma comment(lib, "hlsl-grimoire-sample_miniEngine.lib")

namespace {
	void createRenderTarget(ID3D12Device* device);
	void createDepthRenderTarget(ID3D12Device* device);
	void handleInputInternal();
	void drawInternal();
	ID3D12GraphicsCommandList* getCommandList();

	constexpr DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr UINT kRenderTargetWidth = 1920;
	constexpr UINT kRenderTargetHeight = 1080;
	constexpr float kRenderTargetClearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	constexpr FLOAT kDepthClearVal = 1.0f;
	constexpr UINT kStencilClearVal = 0;

	Models s_models;

	ComPtr<ID3D12Resource> s_renderTarget = nullptr;
	ComPtr<ID3D12DescriptorHeap> s_descHeapRt = nullptr;
	ComPtr<ID3D12Resource> s_depthRenderTarget = nullptr;
	ComPtr<ID3D12DescriptorHeap> s_descHeapDrt = nullptr;
}

namespace MiniEngineIf {
	void init(ID3D12Device* device, HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight)
	{
		g_engine = new TkEngine;
		Dbg::assert_(g_engine != nullptr);
		g_engine->Init(hwnd, frameBufferWidth, frameBufferHeight);

		createRenderTarget(device);
		createDepthRenderTarget(device);
	}

	void end()
	{
		s_models.releaseResource();

		if (g_engine == nullptr)
			return;

		delete g_engine;
		g_engine = nullptr;
	}

	void loadModel()
	{
		s_models.loadModel(g_graphicsEngine->GetRenderContext());
	}

	ID3D12Resource* getRenderTargetResource()
	{
		return s_renderTarget.Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE getOffscreenRtvCpuDescHandle()
	{
		return s_descHeapRt->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE getOffscreenDsvCpuDescHandle()
	{
		return s_descHeapDrt->GetCPUDescriptorHandleForHeapStart();
	}

	void beginFrame(Timestamp* timestamp)
	{
		g_engine->BeginFrame();

		if (timestamp)
		{
			timestamp->query(getCommandList(), Timestamp::Point::kFrameBegin);
		}
	}

	void endFrame(Timestamp* timestamp)
	{
		if (timestamp)
		{
			timestamp->query(getCommandList(), Timestamp::Point::kFrameEnd);
			timestamp->resolve(getCommandList());
		}

		g_engine->EndFrame();
	}

	void clearRenderTarget(ID3D12GraphicsCommandList* commandList)
	{
		commandList->ClearRenderTargetView(s_descHeapRt->GetCPUDescriptorHandleForHeapStart(), kRenderTargetClearColor, 0, nullptr);
	}

	void clearDepthRenderTarget(ID3D12GraphicsCommandList* commandList)
	{
		commandList->ClearDepthStencilView(
			s_descHeapDrt->GetCPUDescriptorHandleForHeapStart(),
            D3D12_CLEAR_FLAG_DEPTH,
            kDepthClearVal,
            kStencilClearVal,
            0,
            nullptr);
	}

	void setOffscreenRenderTarget()
	{
		auto& renderContext = g_graphicsEngine->GetRenderContext();
		renderContext.SetRenderTarget(s_descHeapRt->GetCPUDescriptorHandleForHeapStart(), s_descHeapDrt->GetCPUDescriptorHandleForHeapStart());

		CD3DX12_VIEWPORT vp(s_renderTarget.Get());
		renderContext.SetViewportAndScissor(vp);
	}

	void handleInput()
	{
		handleInputInternal();
	}

	void draw(bool renderToOffscreenBuffer)
	{
		if (renderToOffscreenBuffer)
		{
			setOffscreenRenderTarget();
		}

		drawInternal();
	}

	float getStick(StickType type)
	{
		switch (type) {
		case StickType::kLX: return g_pad[0]->GetLStickXF();
		case StickType::kLY: return g_pad[0]->GetLStickYF();
		case StickType::kRX: return g_pad[0]->GetRStickXF();
		case StickType::kRY: return g_pad[0]->GetRStickYF();
		default: break;
		}

		return 0.0f;
	}

	bool isPress(Button button)
	{
		return g_pad[0]->IsPress(static_cast<EnButton>(button));
	}

	bool isTrigger(Button button)
	{
		return g_pad[0]->IsTrigger(static_cast<EnButton>(button));
	}

	Camera* getCamera2D()
	{
		return g_camera2D;
	}

	Camera* getCamera3D()
	{
		return g_camera3D;
	}
}

namespace {
	void createRenderTarget(ID3D12Device* device)
	{
		{
			const CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(kRenderTargetFormat, kRenderTargetWidth, kRenderTargetHeight);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			const D3D12_CLEAR_VALUE clearVal = {
				.Format = kRenderTargetFormat,
				.Color = { kRenderTargetClearColor[0],  kRenderTargetClearColor[1], kRenderTargetClearColor[2], kRenderTargetClearColor[3]},
			};

			Dbg::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				&clearVal,
				IID_PPV_ARGS(s_renderTarget.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_renderTarget->SetName(L"MiniEngine_RenderTarget"));
		}

		{
			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0,
			};

			Dbg::ThrowIfFailed(device->CreateDescriptorHeap(
				&heapDesc,
				IID_PPV_ARGS(s_descHeapRt.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_descHeapRt->SetName(L"MiniEngine_DescHeapRenderTarget"));

			const D3D12_RENDER_TARGET_VIEW_DESC rtViewDesc = {
				.Format = kRenderTargetFormat,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = { 0, 0 },
			};
			const auto handle = s_descHeapRt->GetCPUDescriptorHandleForHeapStart();

			device->CreateRenderTargetView(
				s_renderTarget.Get(),
				&rtViewDesc,
				handle);
		}
	}

	void createDepthRenderTarget(ID3D12Device* device)
	{
		{
			const CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, kRenderTargetWidth, kRenderTargetHeight);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			const D3D12_CLEAR_VALUE clearVal = {
				.Format = resourceDesc.Format,
				.DepthStencil = { kDepthClearVal, kStencilClearVal },
			};

			Dbg::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearVal,
				IID_PPV_ARGS(s_depthRenderTarget.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_depthRenderTarget->SetName(L"MiniEngine_DepthRenderTarget"));
		}

		{
			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0,
			};

			Dbg::ThrowIfFailed(device->CreateDescriptorHeap(
				&heapDesc,
				IID_PPV_ARGS(s_descHeapDrt.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_descHeapDrt->SetName(L"MiniEngine_DescHeapDepthRenderTarget"));

			const D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {
				.Format = s_depthRenderTarget->GetDesc().Format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
				.Flags = D3D12_DSV_FLAG_NONE,
				.Texture2D = { 0 },
			};
			const auto handle = s_descHeapDrt->GetCPUDescriptorHandleForHeapStart();

			device->CreateDepthStencilView(
				s_depthRenderTarget.Get(),
				&viewDesc,
				handle);
		}
	}

	void handleInputInternal()
	{
		s_models.handleInput();
	}

	void drawInternal()
	{
		auto& renderContext = g_graphicsEngine->GetRenderContext();
		s_models.draw(renderContext);
	}

	ID3D12GraphicsCommandList* getCommandList()
	{
		return g_graphicsEngine->GetCommandList();
	}
}

