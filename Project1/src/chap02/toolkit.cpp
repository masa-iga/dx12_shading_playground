#include "toolkit.h"
#include <array>
#include <d3dx12.h>
#include "config.h"
#include "util.h"

using namespace Microsoft::WRL;

namespace {
	const CD3DX12_VIEWPORT kViewPort(0.0f, 0.0f, static_cast<float>(Config::kRenderTargetWidth), static_cast<float>(Config::kRenderTargetHeight));
	const CD3DX12_RECT kScissorRect(0, 0, Config::kRenderTargetWidth, Config::kRenderTargetHeight);

	enum class Index {
		kCopyTex,
		kLen,
	};
	std::array<LPCWSTR, static_cast<size_t>(Index::kLen)> kShaderFiles = { L"texCopy.hlsl" };
	std::array<LPCSTR, static_cast<size_t>(Index::kLen)> kVsEntrypoints = { "vsmain" };
	std::array<LPCSTR, static_cast<size_t>(Index::kLen)> kPsEntrypoints = { "psmain" };
	std::array<ComPtr<ID3D12RootSignature>, static_cast<size_t>(Index::kLen)> s_rootSignatures = { nullptr };
	std::array<ComPtr<ID3D12PipelineState>, static_cast<size_t>(Index::kLen)> s_pipelineStates = { nullptr };
	std::array<ComPtr<ID3DBlob>, static_cast<size_t>(Index::kLen)> s_vsShaderBlobs = { nullptr };
	std::array<ComPtr<ID3DBlob>, static_cast<size_t>(Index::kLen)> s_psShaderBlobs = { nullptr };
}

namespace Toolkit {
	void init()
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

		// TODO: impl
	}

	void copyTextureToTarget(ID3D12GraphicsCommandList* list)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { };

		list->RSSetViewports(1, &kViewPort);
		list->RSSetScissorRects(1, &kScissorRect);
		list->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

//		list->SetPipelineState(nullptr);
//		list->SetGraphicsRootSignature(getRootSignature());
//		ID3D12DescriptorHeap* const heaps[] = { m_descHeap.Get(), };
//		list->SetDescriptorHeaps(_countof(heaps), heaps);
//		list->SetGraphicsRootDescriptorTable(0, m_descHeap->GetGPUDescriptorHandleForHeapStart());
//
//		list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//		list->IASetVertexBuffers(0, 1, getVertexBufferView());
//		list->DrawInstanced(3, 1, 0, 0);
	}
}