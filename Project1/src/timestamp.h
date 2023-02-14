#pragma once
#include <cstdint>
#include <d3d12.h>
#include <mutex>
#include <wrl.h>

class Timestamp {
public:
	enum class Point {
		kFrameBegin,
		kFrameEnd,
	};
	Timestamp() { }
	~Timestamp() { }
	void init(ID3D12Device* device);
	void setGpuFreq(ID3D12CommandQueue* queue);
	void flip();
	void query(ID3D12GraphicsCommandList* list, Point point);
	void resolve(ID3D12GraphicsCommandList* list);
	double computeDiffInUsec(Point src, Point dst) const;

private:
	static constexpr uint32_t kBuffer = 2;
	static constexpr uint32_t kCount = static_cast<uint32_t>(Point::kFrameEnd) + 1;
	uint32_t m_index = kBuffer - 1;
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_queryHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource = nullptr;
	uint64_t m_gpuFreq = 0;
};

