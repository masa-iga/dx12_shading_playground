#include "render_d3d12.h"
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "debug_win.h"

#pragma comment(lib, "D3DCompiler.lib")

using namespace Microsoft::WRL;

static const D3D_ROOT_SIGNATURE_VERSION kRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1;
static const UINT kCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
static const LPCWSTR kShaderFile = L"simple.hlsl";
static const LPCSTR kVsVersion = "vs_5_1";
static const LPCSTR kPsVersion = "ps_5_1";
static const LPCSTR kVsEntryPoint = "vsmain";
static const LPCSTR kPsEntryPoint = "psmain";

static ComPtr<ID3D12CommandAllocator> s_commandAllocator = nullptr;
static ComPtr<ID3D12RootSignature> s_rootSignature = nullptr;
static ComPtr<ID3D12PipelineState> s_pipelineState = nullptr;

namespace Render {
	void setup(ID3D12Device* device)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(s_commandAllocator.ReleaseAndGetAddressOf()));
	}

	void loadAssets(ID3D12Device* device)
	{
		// create an empty root signature
		{
			const CD3DX12_ROOT_SIGNATURE_DESC desc(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ComPtr<ID3DBlob> signature = nullptr;
			ComPtr<ID3DBlob> error = nullptr;

			Dbg::ThrowIfFailed(D3D12SerializeRootSignature(&desc, kRootSignatureVersion, signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()));
			Dbg::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(s_rootSignature.ReleaseAndGetAddressOf())));
		}

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

		ComPtr<ID3D12GraphicsCommandList> s_commandList = nullptr;
		Dbg::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, s_commandAllocator.Get(), s_pipelineState.Get(), IID_PPV_ARGS(s_commandList.ReleaseAndGetAddressOf())));

		Dbg::ThrowIfFailed(s_commandList.Get()->Close());

//		// create the vertex buffer
//		{
//			struct Vertex
//			{
//				DirectX::XMFLOAT3 position = { };
//				DirectX::XMFLOAT4 color = { };
//			};
//
//			float m_aspectRatio = 1.0f;
//
//			const Vertex triangleVertices[] = {
//				{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
//				{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
//				{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
//			};
//		}
	}
}
