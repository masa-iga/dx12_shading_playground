#include "timestamp.h"
#include <d3dx12.h>
#include "debug_win.h"
using namespace Microsoft::WRL;

void Timestamp::init(ID3D12Device* device)
{
	{
		const D3D12_QUERY_HEAP_DESC desc = {
			.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
			.Count = kBuffer * kCount,
			.NodeMask = 0,
		};

		auto hr = device->CreateQueryHeap(
			&desc,
			IID_PPV_ARGS(m_queryHeap.ReleaseAndGetAddressOf()));
		Dbg::ThrowIfFailed(hr);
		Dbg::ThrowIfFailed(m_queryHeap->SetName(L"QueryHeapForTimestamp"));
	}

	{
		const auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint64_t) * kBuffer * kCount);

		auto hr = device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(m_resource.ReleaseAndGetAddressOf()));
		Dbg::ThrowIfFailed(hr);
		Dbg::ThrowIfFailed(m_resource->SetName(L"timestamp"));
	}
}

void Timestamp::setGpuFreq(ID3D12CommandQueue* queue)
{
	auto hr = queue->GetTimestampFrequency(&m_gpuFreq);
	Dbg::ThrowIfFailed(hr);
}

void Timestamp::flip()
{
	m_index = (m_index + 1) % kBuffer;
}

void Timestamp::query(ID3D12GraphicsCommandList* list, Point point)
{
	const UINT index = (kCount * m_index) + static_cast<UINT>(point);

	list->EndQuery(
		m_queryHeap.Get(),
		D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP,
		index);
}

void Timestamp::resolve(ID3D12GraphicsCommandList* list)
{
	const UINT startIndex = kCount * m_index;
	const UINT numQueries = kCount;
	const UINT64 alignedDestinationBufferOffset = static_cast<UINT64>(kCount) * m_index * sizeof(uint64_t);

	list->ResolveQueryData(
		m_queryHeap.Get(),
		D3D12_QUERY_TYPE_TIMESTAMP,
		startIndex,
		numQueries,
		m_resource.Get(),
		alignedDestinationBufferOffset);
}

double Timestamp::computeDiffInUsec(Point src, Point dst) const
{
	const D3D12_RANGE range = {
		.Begin = static_cast<SIZE_T>(kCount) * m_index,
		.End = static_cast<SIZE_T>(kCount) * (static_cast<SIZE_T>(m_index) + 1) - 1,
	};
	uint64_t* pData = nullptr;

	Dbg::ThrowIfFailed(m_resource->Map(0, &range, reinterpret_cast<void**>(&pData)));
	m_resource->Unmap(0, nullptr);

	const uint64_t diff = pData[static_cast<size_t>(dst)] - pData[static_cast<size_t>(src)];

	return diff * 1'000'000 / static_cast<double>(m_gpuFreq);
}
