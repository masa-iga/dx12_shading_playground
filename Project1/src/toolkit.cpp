#include "toolkit.h"
#include <array>
#include <d3dx12.h>
#include <directxmath.h>
#include "config.h"
#include "debug_win.h"
#include "device_d3d12.h"
#include "util.h"

using namespace Microsoft::WRL;

namespace {
	const CD3DX12_VIEWPORT kViewPort(0.0f, 0.0f, static_cast<float>(Config::kRenderTargetWidth), static_cast<float>(Config::kRenderTargetHeight));
	const CD3DX12_RECT kScissorRect(0, 0, Config::kRenderTargetWidth, Config::kRenderTargetHeight);
	void initTexCopy(ID3D12Device* device);

	constexpr DXGI_FORMAT kRtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	enum class Index {
		kCopyTex,
		kLen,
	};
	ID3D12Device* s_device = nullptr;
	constexpr std::array<LPCWSTR, static_cast<size_t>(Index::kLen)> kShaderFiles = { L"texCopy.hlsl" };
	constexpr std::array<LPCSTR, static_cast<size_t>(Index::kLen)> kVsEntrypoints = { "vsmain" };
	constexpr std::array<LPCSTR, static_cast<size_t>(Index::kLen)> kPsEntrypoints = { "psmain" };
	std::array<ComPtr<ID3DBlob>, static_cast<size_t>(Index::kLen)> s_vsShaderBlobs = { nullptr };
	std::array<ComPtr<ID3DBlob>, static_cast<size_t>(Index::kLen)> s_psShaderBlobs = { nullptr };
	std::array<ComPtr<ID3D12RootSignature>, static_cast<size_t>(Index::kLen)> s_rootSignatures = { nullptr };
	std::array<ComPtr<ID3D12PipelineState>, static_cast<size_t>(Index::kLen)> s_pipelineStates = { nullptr };
	std::array<ComPtr<ID3D12Resource>, static_cast<size_t>(Index::kLen)> s_vertexBuffers = { nullptr };
	std::array<D3D12_VERTEX_BUFFER_VIEW, static_cast<size_t>(Index::kLen)> s_vbViews = { { } };
	std::array<ComPtr<ID3D12DescriptorHeap>, static_cast<size_t>(Index::kLen)> s_descHeaps = { nullptr };
}

namespace {
	enum class TexCopyHeapIndex : UINT {
		kCbvWorldMatrix,
		kSrvSrcTex,
	};
	ComPtr<ID3D12Resource> s_texCopyMatrixBuffer = nullptr;
	void updateTexCopyDescHeapToRegisterSrv(ID3D12Device* device, ID3D12Resource* srcTexResource);
}

namespace Toolkit {
	void init(ID3D12Device* device)
	{
		s_device = device;
		initTexCopy(s_device);
	}

	void copyTextureToTarget(ID3D12GraphicsCommandList* list, ID3D12Resource* srcTexResource)
	{
		updateTexCopyDescHeapToRegisterSrv(s_device, srcTexResource);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { };

		list->RSSetViewports(1, &kViewPort); // TODO: needed ?
		list->RSSetScissorRects(1, &kScissorRect); // TODO: needed ?
//		list->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		list->SetPipelineState(s_pipelineStates.at(static_cast<uint32_t>(Index::kCopyTex)).Get());
		list->SetGraphicsRootSignature(s_rootSignatures.at(static_cast<uint32_t>(Index::kCopyTex)).Get());
		ID3D12DescriptorHeap* const heaps[] = { s_descHeaps.at(static_cast<uint32_t>(Index::kCopyTex)).Get(), };
		list->SetDescriptorHeaps(_countof(heaps), heaps);
		list->SetGraphicsRootDescriptorTable(0, s_descHeaps.at(static_cast<uint32_t>(Index::kCopyTex))->GetGPUDescriptorHandleForHeapStart());

		list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		list->IASetVertexBuffers(0, 1, &s_vbViews.at(static_cast<uint32_t>(Index::kCopyTex)));
		list->DrawInstanced(4, 1, 0, 0);
	}
}

namespace {
	void initTexCopy(ID3D12Device* device)
	{
		Util::compileVsShader(
			s_vsShaderBlobs.at(static_cast<uint32_t>(Index::kCopyTex)),
			kShaderFiles.at(static_cast<uint32_t>(Index::kCopyTex)),
			kVsEntrypoints.at(static_cast<uint32_t>(Index::kCopyTex))
		);
		Util::compilePsShader(
			s_psShaderBlobs.at(static_cast<uint32_t>(Index::kCopyTex)),
			kShaderFiles.at(static_cast<uint32_t>(Index::kCopyTex)),
			kPsEntrypoints.at(static_cast<uint32_t>(Index::kCopyTex))
		);

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
			Dbg::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(s_rootSignatures.at(static_cast<uint32_t>(Index::kCopyTex)).ReleaseAndGetAddressOf())));
		}

		{
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
				.pRootSignature = s_rootSignatures.at(static_cast<uint32_t>(Index::kCopyTex)).Get(),
				.VS = { s_vsShaderBlobs.at(static_cast<uint32_t>(Index::kCopyTex))->GetBufferPointer(), s_vsShaderBlobs.at(static_cast<uint32_t>(Index::kCopyTex))->GetBufferSize() },
				.PS = { s_psShaderBlobs.at(static_cast<uint32_t>(Index::kCopyTex))->GetBufferPointer(), s_psShaderBlobs.at(static_cast<uint32_t>(Index::kCopyTex))->GetBufferSize() },
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
			psoDesc.RTVFormats[0] = kRtvFormat;

			Dbg::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(s_pipelineStates.at(static_cast<uint32_t>(Index::kCopyTex)).ReleaseAndGetAddressOf())));
		}

		struct Vertex
		{
			DirectX::XMFLOAT3 position = { };
			DirectX::XMFLOAT2 uv = { };
		};

		const Vertex triangleVertices[] = {
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
			{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
		};
		constexpr UINT vertexBufferSize = sizeof(triangleVertices);

		{
			const CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			const auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

			Dbg::ThrowIfFailed(device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(s_vertexBuffers.at(static_cast<uint32_t>(Index::kCopyTex)).ReleaseAndGetAddressOf())));
		}

		{
			UINT8* pVertexDataBegin = nullptr;
			const CD3DX12_RANGE readRange(0, 0);

			Dbg::ThrowIfFailed(s_vertexBuffers.at(static_cast<uint32_t>(Index::kCopyTex))->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			{
				std::memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
			}
			s_vertexBuffers.at(static_cast<uint32_t>(Index::kCopyTex))->Unmap(0, nullptr);
		}

		{
			s_vbViews.at(static_cast<uint32_t>(Index::kCopyTex)).BufferLocation = s_vertexBuffers.at(static_cast<uint32_t>(Index::kCopyTex))->GetGPUVirtualAddress();
			s_vbViews.at(static_cast<uint32_t>(Index::kCopyTex)).SizeInBytes = vertexBufferSize;
			s_vbViews.at(static_cast<uint32_t>(Index::kCopyTex)).StrideInBytes = sizeof(Vertex);
		}

		{
			constexpr UINT kNumOfDesc = 2;

			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				.NumDescriptors = kNumOfDesc,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				.NodeMask = 1,
			};
			Dbg::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(s_descHeaps.at(static_cast<uint32_t>(Index::kCopyTex)).ReleaseAndGetAddressOf())));
		}

		{
			CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Util::getAlignedContantBufferSize(sizeof(DirectX::XMMATRIX)));

			Dbg::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(s_texCopyMatrixBuffer.ReleaseAndGetAddressOf()))
			);
		}

		{
			using namespace DirectX;

			XMMATRIX* pMatrix = nullptr;
			const CD3DX12_RANGE readRange(0, 0);

			Dbg::ThrowIfFailed(s_texCopyMatrixBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pMatrix)));
			{
				constexpr float scale[] = { 0.5f, 0.5f, 0.5f };
				constexpr float trans[] = { -0.45f, -0.45f, 0.0f };
				*pMatrix = DirectX::XMMatrixMultiply(
					DirectX::XMMatrixScaling(scale[0], scale[1], scale[2]),
					DirectX::XMMatrixTranslation(trans[0], trans[1], trans[2])
				);
			}
			s_texCopyMatrixBuffer->Unmap(0, nullptr);
		}

		{
			const D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
				.BufferLocation = s_texCopyMatrixBuffer->GetGPUVirtualAddress(),
				.SizeInBytes = static_cast<UINT>(s_texCopyMatrixBuffer->GetDesc().Width),
			};
			const CD3DX12_CPU_DESCRIPTOR_HANDLE destDesc(
				s_descHeaps.at(static_cast<uint32_t>(Index::kCopyTex))->GetCPUDescriptorHandleForHeapStart(),
				static_cast<INT>(TexCopyHeapIndex::kCbvWorldMatrix),
				DeviceD3D12::getDescHandleIncSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			);

			device->CreateConstantBufferView(&desc, destDesc);
		}
	}
}

namespace {
	void updateTexCopyDescHeapToRegisterSrv(ID3D12Device* device, ID3D12Resource* srcTexResource)
	{
		Dbg::assert_(srcTexResource != nullptr);
		Dbg::assert_(srcTexResource->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);

		const D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
			.Format = srcTexResource->GetDesc().Format,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = {
				.MostDetailedMip = 0,
				.MipLevels = 1,
				.PlaneSlice = 0,
				.ResourceMinLODClamp = 0,
			},
		};

		const CD3DX12_CPU_DESCRIPTOR_HANDLE destDesc(
			s_descHeaps.at(static_cast<uint32_t>(Index::kCopyTex))->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(TexCopyHeapIndex::kSrvSrcTex),
			DeviceD3D12::getDescHandleIncSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);

		device->CreateShaderResourceView(
            srcTexResource,
            &desc,
            destDesc);
	}
}