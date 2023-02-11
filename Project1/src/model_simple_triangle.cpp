#include "model_simple_triangle.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <typeinfo>
#include "config.h"
#include "dds_loader_if.h"
#include "debug_win.h"
#include "device_d3d12.h"
#include "util.h"

using namespace Microsoft::WRL;

constexpr float kTrans[] = { 0.25f, 0.25f, 0.0f };
constexpr wchar_t kDdsFileName[] = L"../import/hlsl-grimoire-sample/Sample_03_02/Sample_03_02/Assets/image/sample_00.dds";

void SimpleTriangleModel::createResource(ID3D12Device* device)
{
	createDescHeap(device);
	createGraphicsPipelineState(device);
	createVertex(device);
	createWorldMatrix(device);
}

void SimpleTriangleModel::uploadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* list)
{
	createTexture(device, list);
}

void SimpleTriangleModel::releaseTemporaryBuffers()
{
	m_uploadTexture.Reset();
}

void SimpleTriangleModel::draw(ID3D12GraphicsCommandList* list)
{
	list->SetPipelineState(getPipelineState());
	list->SetGraphicsRootSignature(getRootSignature());
	ID3D12DescriptorHeap* const heaps[] = { m_descHeap.Get(), };
	list->SetDescriptorHeaps(_countof(heaps), heaps);
	list->SetGraphicsRootDescriptorTable(0, m_descHeap->GetGPUDescriptorHandleForHeapStart());

	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->IASetVertexBuffers(0, 1, getVertexBufferView());
	list->DrawInstanced(3, 1, 0, 0);
}

void SimpleTriangleModel::createGraphicsPipelineState(ID3D12Device* device)
{
	ComPtr<ID3DBlob> vertexShader = nullptr;
	ComPtr<ID3DBlob> pixelShader = nullptr;

	// create the pipline state, which includes compiling and loading shaders
	{
		Dbg::ThrowIfFailed(Util::compileVsShader(vertexShader, kShaderFile, kVsEntryPoint));
		Dbg::ThrowIfFailed(Util::compilePsShader(pixelShader, kShaderFile, kPsEntryPoint));
	}

	// create root signature
	{
		const D3D12_DESCRIPTOR_RANGE descRanges[] = {
			CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0),
			CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0),
		};

		CD3DX12_ROOT_PARAMETER rootParam;
		rootParam.InitAsDescriptorTable(_countof(descRanges), descRanges);
		const CD3DX12_STATIC_SAMPLER_DESC sampleDesc(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

		const CD3DX12_ROOT_SIGNATURE_DESC desc(1, &rootParam, 1, &sampleDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature = nullptr;
		ComPtr<ID3DBlob> error = nullptr;

		Dbg::ThrowIfFailed(D3D12SerializeRootSignature(&desc, Config::kRootSignatureVersion, signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()));
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
				.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0,
			},
			{
				.SemanticName = "COLOR",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0,
			},
			{
				.SemanticName = "TEXCOORD",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
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
		DirectX::XMFLOAT2 uv = { };
	};

	const Vertex triangleVertices[] = {
		{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } , { 0.5f, 0.0f } },
		{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
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

void SimpleTriangleModel::createWorldMatrix(ID3D12Device* device)
{
	{
		CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Util::getAlignedContantBufferSize(sizeof(DirectX::XMMATRIX)));

		Dbg::ThrowIfFailed(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_resourceWorldMatrix.ReleaseAndGetAddressOf()))
		);
	}
	
	{
		using namespace DirectX;

		XMMATRIX* pMatrix = nullptr;
		const CD3DX12_RANGE readRange(0, 0);

		Dbg::ThrowIfFailed(m_resourceWorldMatrix->Map(0, &readRange, reinterpret_cast<void**>(&pMatrix)));
		{
			*pMatrix = DirectX::XMMatrixTranslation(kTrans[0], kTrans[1], kTrans[2]);
		}
		m_resourceWorldMatrix->Unmap(0, nullptr);
	}

	{
		const D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
			.BufferLocation = m_resourceWorldMatrix->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UINT>(m_resourceWorldMatrix->GetDesc().Width),
		};
		const CD3DX12_CPU_DESCRIPTOR_HANDLE destDesc(
			m_descHeap->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(DescHeapIndex::kCbvWorldMatrix),
			DeviceD3D12::getDescHandleIncSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);

		device->CreateConstantBufferView(&desc, destDesc);
	}
}

void SimpleTriangleModel::createTexture(ID3D12Device* device, ID3D12GraphicsCommandList* list)
{
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;

	Dbg::ThrowIfFailed(DdsLoaderIf::LoadDDSTextureFromFile(kDdsFileName, m_texture.ReleaseAndGetAddressOf(), ddsData, subresources));
	Dbg::ThrowIfFailed(m_texture->SetName(L"texture"));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, static_cast<UINT>(subresources.size()));

#define DEBUG_PRINT (0)
#if DEBUG_PRINT
	{
		const auto resource = m_texture.Get();
		Dbg::print("%s::%s()\n", typeid(*this).name(), __func__);
		Dbg::print("  size %zd\n", uploadBufferSize);
		Util::debugPrint(m_texture.Get());
	}
#endif // #if DEBUG_PRINT
#undef DEBUG_PRINT

	{
		const CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		const auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		Dbg::ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_uploadTexture.GetAddressOf())));
		Dbg::ThrowIfFailed(m_uploadTexture->SetName(L"uploadTextuer"));
	}

#define DEBUG_PRINT (0)
#if DEBUG_PRINT
	{
		Dbg::print("%s::%s()\n", typeid(*this).name(), __func__);
		Util::debugPrint(m_uploadTexture.Get());
	}
#endif // #if DEBUG_PRINT
#undef DEBUG_PRINT

	{
		const UINT64 intermediateOffset = 0;
		const UINT firstSubresource = 0;
		UpdateSubresources(list, m_texture.Get(), m_uploadTexture.Get(), intermediateOffset, firstSubresource, static_cast<UINT>(subresources.size()), subresources.data());
	}

	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		list->ResourceBarrier(1, &barrier);
	}

	{
		const D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
			.Format = m_texture->GetDesc().Format,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = {
				.MostDetailedMip = 0,
				.MipLevels = m_texture->GetDesc().MipLevels,
				.PlaneSlice = 0,
				.ResourceMinLODClamp = 0,
			},
		};

		const CD3DX12_CPU_DESCRIPTOR_HANDLE destDesc(
			m_descHeap->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(DescHeapIndex::kSrvTexture),
			DeviceD3D12::getDescHandleIncSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);

		device->CreateShaderResourceView(m_texture.Get(), &desc, destDesc);
	}
}

void SimpleTriangleModel::createDescHeap(ID3D12Device* device)
{
	constexpr UINT kNumOfDesc = 2;

	const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = kNumOfDesc,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		.NodeMask = 1,
	};
	Dbg::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descHeap.ReleaseAndGetAddressOf())));
}

