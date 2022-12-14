#include "render_d3d12.h"
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "debug_win.h"
#include "swapchain_d3d12.h"

#pragma comment(lib, "D3DCompiler.lib")

using namespace Microsoft::WRL;

static const D3D_ROOT_SIGNATURE_VERSION kRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1;
static const UINT kCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
static const LPCWSTR kShaderFile = L"simple.hlsl";
static const LPCSTR kVsVersion = "vs_5_1";
static const LPCSTR kPsVersion = "ps_5_1";
static const LPCSTR kVsEntryPoint = "vsmain";
static const LPCSTR kPsEntryPoint = "psmain";

static ComPtr<ID3D12CommandQueue> s_commandQueue = nullptr;
static ComPtr<ID3D12CommandAllocator> s_commandAllocator = nullptr;
static ComPtr<ID3D12GraphicsCommandList> s_commandList = nullptr;
static ComPtr<ID3D12RootSignature> s_rootSignature = nullptr;
static ComPtr<ID3D12PipelineState> s_pipelineState = nullptr;
static ComPtr<ID3D12Resource> s_vertexBuffer = nullptr;
static D3D12_VERTEX_BUFFER_VIEW s_vbView = { };
static ComPtr<ID3D12Fence> s_fence = nullptr;
static UINT64 s_fenceValue = 0;
static HANDLE s_fenceEvent = nullptr;
static UINT s_frameIndex = 0;

static void createCommandAllocator(ID3D12Device* device);
static void createCommandQueue(ID3D12Device* device);
static void createCommandList(ID3D12Device* device);
static void createGraphicsPipelineState(ID3D12Device* device);
static void createVertex(ID3D12Device* device);
static void createFence(ID3D12Device* device);
static void waitForPreviousFrame();
static void populateCommandList();

namespace Render {
	void setup(ID3D12Device* device)
	{
		createCommandAllocator(device);
		createCommandQueue(device);
	}

	void loadAssets(ID3D12Device* device)
	{
		s_frameIndex = SwapChain::getSwapChain()->GetCurrentBackBufferIndex();
		createGraphicsPipelineState(device);
		createCommandList(device);
		createVertex(device);
		createFence(device);
		waitForPreviousFrame();
	}

	void onUpdate()
	{
		;
	}

	void onRender()
	{
		populateCommandList();

		ID3D12CommandList* ppCommandLists[] = { s_commandList.Get() };
		s_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		{
			const UINT syncInterval = 1;
			const UINT flags = 0;
			Dbg::ThrowIfFailed(SwapChain::getSwapChain()->Present(syncInterval, flags));

			waitForPreviousFrame();
		}
	}

	ID3D12CommandQueue* getCommandQueue()
	{
		return s_commandQueue.Get();
	}
}

void createCommandList(ID3D12Device* device)
{
	Dbg::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, s_commandAllocator.Get(), nullptr, IID_PPV_ARGS(s_commandList.ReleaseAndGetAddressOf())));

	Dbg::ThrowIfFailed(s_commandList.Get()->Close());
}

void createCommandAllocator(ID3D12Device* device)
{
	Dbg::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(s_commandAllocator.ReleaseAndGetAddressOf())));
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
}

void createGraphicsPipelineState(ID3D12Device* device)
{
	ComPtr<ID3DBlob> vertexShader = nullptr;
	ComPtr<ID3DBlob> pixelShader = nullptr;

	// create the pipline state, which includes compiling and loading shaders
	{
		ComPtr<ID3DBlob> error = nullptr;

		auto result = D3DCompileFromFile(kShaderFile, nullptr, nullptr, kVsEntryPoint, kVsVersion, kCompileFlags, 0, vertexShader.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf());

		if (FAILED(result))
		{
			Dbg::printBlob(error.Get());
			Dbg::ThrowIfFailed(E_FAIL);
		}

		result = D3DCompileFromFile(kShaderFile, nullptr, nullptr, kPsEntryPoint, kPsVersion, kCompileFlags, 0, pixelShader.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf());

		if (FAILED(result))
		{
			Dbg::printBlob(error.Get());
			Dbg::ThrowIfFailed(E_FAIL);
		}
	}

	// create an empty root signature
	{
		const CD3DX12_ROOT_SIGNATURE_DESC desc(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature = nullptr;
		ComPtr<ID3DBlob> error = nullptr;

		Dbg::ThrowIfFailed(D3D12SerializeRootSignature(&desc, kRootSignatureVersion, signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()));
		Dbg::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(s_rootSignature.ReleaseAndGetAddressOf())));
	}

	{
		// define the vertex input layout
		constexpr D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{
				.SemanticName = "POSITION",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = 0,
				.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0,
			},
			{
				.SemanticName = "COLOR",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = 12,
				.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0,
			},
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {
			.pRootSignature = s_rootSignature.Get(),
			.VS = { vertexShader.Get()->GetBufferPointer(), vertexShader.Get()->GetBufferSize() },
			.PS = { pixelShader.Get()->GetBufferPointer(), pixelShader.Get()->GetBufferSize() },
			.DS = { nullptr, 0 },
			.HS = { nullptr, 0 },
			.GS = { nullptr, 0 },
			.StreamOutput = { },
			.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT()),
			.SampleMask = UINT_MAX,
			.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT()),
			.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT()),
			.InputLayout = { inputElementDescs, _countof(inputElementDescs)},
			.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.NumRenderTargets = 1,
			.RTVFormats = { },
			.DSVFormat = DXGI_FORMAT_UNKNOWN,
			.SampleDesc = { 1, 0 },
			.NodeMask = 0,
			.CachedPSO = { nullptr, 0 },
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
		};
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DepthStencilState.StencilEnable = false;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		Dbg::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(s_pipelineState.ReleaseAndGetAddressOf())));
	}
}

void createVertex(ID3D12Device* device)
{
	const float m_aspectRatio = 1.0f;

	struct Vertex
	{
		DirectX::XMFLOAT3 position = { };
		DirectX::XMFLOAT4 color = { };
	};

	const Vertex triangleVertices[] = {
		{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	};
	const UINT vertexBufferSize = sizeof(triangleVertices);

	{
		const CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		const auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

		Dbg::ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(s_vertexBuffer.ReleaseAndGetAddressOf())));
	}

	{
		UINT8* pVertexDataBegin = nullptr;
		const CD3DX12_RANGE readRange(0, 0);

		Dbg::ThrowIfFailed(s_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		{
			std::memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		}
		s_vertexBuffer->Unmap(0, nullptr);
	}

	{
		s_vbView.BufferLocation = s_vertexBuffer.Get()->GetGPUVirtualAddress();
		s_vbView.SizeInBytes = vertexBufferSize;
		s_vbView.StrideInBytes = sizeof(Vertex);
	}
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

void populateCommandList()
{
	Dbg::ThrowIfFailed(s_commandAllocator->Reset());
	Dbg::ThrowIfFailed(s_commandList->Reset(s_commandAllocator.Get(), nullptr));

	{
		const CD3DX12_VIEWPORT vp(0.0f, 0.0f, 1920.0f, 1080.0f);
		const CD3DX12_RECT scrt(0, 0, 1920, 1080);

		s_commandList->SetPipelineState(s_pipelineState.Get());
		s_commandList->SetGraphicsRootSignature(s_rootSignature.Get());
		s_commandList->RSSetViewports(1, &vp);
		s_commandList->RSSetScissorRects(1, &scrt);

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(SwapChain::getRtResource(s_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		s_commandList->ResourceBarrier(1, &barrier);


		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(SwapChain::getRtvDescHeap()->GetCPUDescriptorHandleForHeapStart(), s_frameIndex, SwapChain::getRtvDescSize());
		s_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		s_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		s_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		s_commandList->IASetVertexBuffers(0, 1, &s_vbView);
		s_commandList->DrawInstanced(3, 1, 0, 0);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(SwapChain::getRtResource(s_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		s_commandList->ResourceBarrier(1, &barrier);
	}

	Dbg::ThrowIfFailed(s_commandList->Close());
}

