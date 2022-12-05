#include "render_d3d12.h"
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <d3dx12.h>
#include "debug_win.h"
#include <wrl/client.h>

#pragma comment(lib, "D3DCompiler.lib")

using namespace Microsoft::WRL;

static const D3D_ROOT_SIGNATURE_VERSION kRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1;
static const UINT kCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
static const LPCSTR kVsVersion = "vs_5_0";
static const LPCWSTR kVertexShader = L"VertexShader.hlsl";
static const LPCSTR kVsEntryPoint = "main";

static ComPtr<ID3D12CommandAllocator> s_commandAllocator = nullptr;
static ComPtr<ID3D12RootSignature> s_rootSignature = nullptr;

namespace Render {
	void setup(ID3D12Device* device)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(s_commandAllocator.ReleaseAndGetAddressOf()));
	}

	void loadAsserts(ID3D12Device* device)
	{
		// create an empty root signature
		{
			const CD3DX12_ROOT_SIGNATURE_DESC desc(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ComPtr<ID3DBlob> signature = nullptr;
			ComPtr<ID3DBlob> error = nullptr;

			Dbg::ThrowIfFailed(D3D12SerializeRootSignature(&desc, kRootSignatureVersion, signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()));
			Dbg::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(s_rootSignature.ReleaseAndGetAddressOf())));
		}

		// create the pipline state, which includes compiling and loading shaders
		{
			ComPtr<ID3DBlob> vertexShader = nullptr;
			ComPtr<ID3DBlob> pixelShader = nullptr;
			ComPtr<ID3DBlob> error = nullptr;

			auto result = D3DCompileFromFile(kVertexShader, nullptr, nullptr, kVsEntryPoint, kVsVersion, kCompileFlags, 0, vertexShader.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf());

			if (FAILED(result))
			{
				Dbg::printBlob(error.Get());
				Dbg::ThrowIfFailed(E_FAIL);
			}
		}
	}
}
