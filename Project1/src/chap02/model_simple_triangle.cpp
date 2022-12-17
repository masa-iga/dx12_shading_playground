#include "model_simple_triangle.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "config.h"
#include "debug_win.h"

using namespace Microsoft::WRL;

void SimpleTriangleModel::createGraphicsPipelineState(ID3D12Device* device)
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
		Dbg::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())));
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
			.pRootSignature = m_rootSignature.Get(),
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

		Dbg::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf())));
	}
}

void SimpleTriangleModel::createVertex(ID3D12Device* device)
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
			IID_PPV_ARGS(m_vertexBuffer.ReleaseAndGetAddressOf())));
	}

	{
		UINT8* pVertexDataBegin = nullptr;
		const CD3DX12_RANGE readRange(0, 0);

		Dbg::ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		{
			std::memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		}
		m_vertexBuffer->Unmap(0, nullptr);
	}

	{
		m_vbView.BufferLocation = m_vertexBuffer.Get()->GetGPUVirtualAddress();
		m_vbView.SizeInBytes = vertexBufferSize;
		m_vbView.StrideInBytes = sizeof(Vertex);
	}
}

