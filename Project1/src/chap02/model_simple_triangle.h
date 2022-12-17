#pragma once
#include <d3d12.h>
#include <wrl/client.h>

class SimpleTriangleModel {
public:
	void createResource(ID3D12Device* device);
	void draw(ID3D12GraphicsCommandList* list);

private:
	static constexpr LPCWSTR kShaderFile = L"simple.hlsl";
	static constexpr LPCSTR kVsEntryPoint = "vsmain";
	static constexpr LPCSTR kPsEntryPoint = "psmain";

	ID3D12RootSignature* getRootSignature() const { return m_rootSignature.Get(); }
	ID3D12PipelineState* getPipelineState() const { return m_pipelineState.Get(); }
	const D3D12_VERTEX_BUFFER_VIEW* getVertexBufferView() const { return &m_vbView; };
	void createGraphicsPipelineState(ID3D12Device* device);
	void createVertex(ID3D12Device* device);
	void createWorldMatrix(ID3D12Device* device);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resourceWorldMatrix = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vbView = { };
};
