#include "toolkit.h"
#include <d3dx12.h>
#include "config.h"

using namespace Microsoft::WRL;

namespace {
	const CD3DX12_VIEWPORT kViewPort(0.0f, 0.0f, static_cast<float>(Config::kRenderTargetWidth), static_cast<float>(Config::kRenderTargetHeight));
	const CD3DX12_RECT kScissorRect(0, 0, Config::kRenderTargetWidth, Config::kRenderTargetHeight);

	ComPtr<ID3D12RootSignature> s_rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> s_pipelineState = nullptr;
}

namespace Toolkit {
	void init()
	{
	}

	void renderTextureToTarget(ID3D12GraphicsCommandList* list)
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