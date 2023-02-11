#include "util.h"
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <d3dcompiler.h>
#endif // #if defined(_WIN32) || defined(_WIN64)
#include "config.h"
#include "debug_win.h"
#pragma warning(push)
#pragma warning(disable: 26812)

#if defined(_WIN32) || defined(_WIN64)
using namespace Microsoft::WRL;
#endif // #if defined(_WIN32) || defined(_WIN64)

namespace {
#if defined(_WIN32) || defined(_WIN64)
	HRESULT compileShader(ComPtr<ID3DBlob>& shaderBlob, LPCWSTR shaderFile, LPCSTR entrypoint, LPCSTR target);
	const char* getNameOfD3d12HeapType(D3D12_HEAP_TYPE type);
	const char* getNameOfD3d12CpuPageProperty(D3D12_CPU_PAGE_PROPERTY prop);
	const char* getNameOfD3d12MemoryPool(D3D12_MEMORY_POOL mpool);
	const char* getNameOfD3d12DxgiFormat(DXGI_FORMAT format);
	const char* getNameOfD3d12TextureLayout(D3D12_TEXTURE_LAYOUT layout);
#endif // #if defined(_WIN32) || defined(_WIN64)
}

namespace Util {
#if defined(_WIN32) || defined(_WIN64)
	HRESULT compileVsShader(ComPtr<ID3DBlob>& shaderBlob, LPCWSTR shaderFile, LPCSTR entrypoint)
	{
		return compileShader(shaderBlob, shaderFile, entrypoint, Config::kVsVersion);
	}

	HRESULT compilePsShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCWSTR shaderFile, LPCSTR entrypoint)
	{
		return compileShader(shaderBlob, shaderFile, entrypoint, Config::kPsVersion);
	}

	void debugPrint(ID3D12Resource* resource)
	{
		const auto d = resource->GetDesc();
		Dbg::print("  desc: dim %d width %zd height %d mip %d slice %d %s %s\n", d.Dimension, d.Width, d.Height, d.MipLevels, d.DepthOrArraySize, getNameOfD3d12DxgiFormat(d.Format), getNameOfD3d12TextureLayout(d.Layout));

		D3D12_HEAP_PROPERTIES hp = { };
		D3D12_HEAP_FLAGS hf = { };
		Dbg::ThrowIfFailed(resource->GetHeapProperties(&hp, &hf));
		Dbg::print("  heap: %s %s %s flag 0x%x\n", getNameOfD3d12HeapType(hp.Type), getNameOfD3d12CpuPageProperty(hp.CPUPageProperty), getNameOfD3d12MemoryPool(hp.MemoryPoolPreference), hf);
	}
#endif // #if defined(_WIN32) || defined(_WIN64)
}

namespace {
#if defined(_WIN32) || defined(_WIN64)
	HRESULT compileShader(ComPtr<ID3DBlob>& shaderBlob, LPCWSTR shaderFile, LPCSTR entrypoint, LPCSTR target)
	{
		ComPtr<ID3DBlob> error = nullptr;

		auto result = D3DCompileFromFile(
			shaderFile,
			nullptr,
			nullptr,
			entrypoint,
			target,
			Config::kCompileFlags1,
			Config::kCompileFlags2,
			shaderBlob.ReleaseAndGetAddressOf(),
			error.ReleaseAndGetAddressOf()
		);

		if (FAILED(result))
		{
			Dbg::printBlob(error.Get());
			return E_FAIL;
		}

		return S_OK;
	}

	const char* getNameOfD3d12HeapType(D3D12_HEAP_TYPE type)
	{
		switch (type) {
		case D3D12_HEAP_TYPE_DEFAULT: return "HEAP_DEFAULT";
		case D3D12_HEAP_TYPE_UPLOAD: return "HEAP_UPLOAD";
		case D3D12_HEAP_TYPE_READBACK: return "HEAP_READBACK";
		case D3D12_HEAP_TYPE_CUSTOM: return "HEAP_CUSTOM";
		}
		return "UNKNOWN !!";
	}

	const char* getNameOfD3d12CpuPageProperty(D3D12_CPU_PAGE_PROPERTY prop)
	{
		switch (prop) {
		case D3D12_CPU_PAGE_PROPERTY_UNKNOWN: return "PAGE_PROP_UNKNOWN";
		case D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE: return "PAGE_PROP_NOT_AVAL";
		case D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE: return "PAGE_PROP_WC";
		case D3D12_CPU_PAGE_PROPERTY_WRITE_BACK: return "PAGE_PROP_WB";
		}
		return "UNKNOWN !!";
	}

	const char* getNameOfD3d12MemoryPool(D3D12_MEMORY_POOL mpool)
	{
		switch (mpool) {
		case D3D12_MEMORY_POOL_UNKNOWN: return "MPOOL_UNKNOWN";
		case D3D12_MEMORY_POOL_L0: return "MPOOL_L0";
		case D3D12_MEMORY_POOL_L1: return "MPOOL_L1";
		};
		return "UNKNOWN !!";
	}

	const char*  getNameOfD3d12DxgiFormat(DXGI_FORMAT format)
	{
		switch (format) {
		case DXGI_FORMAT_UNKNOWN: return "FORMAT_UNKNOWN";
		case DXGI_FORMAT_R32G32B32A32_TYPELESS: return "FORMAT_R32G32B32A32_TYPELESS";
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return "FORMAT_R32G32B32A32_FLOAT";
		case DXGI_FORMAT_R32G32B32A32_UINT: return "FORMAT_R32G32B32A32_UINT";
		case DXGI_FORMAT_R32G32B32A32_SINT: return "FORMAT_R32G32B32A32_SINT";
		case DXGI_FORMAT_R32G32B32_TYPELESS: return "FORMAT_R32G32B32_TYPELESS";
		case DXGI_FORMAT_R32G32B32_FLOAT: return "FORMAT_R32G32B32_FLOAT";
		case DXGI_FORMAT_R32G32B32_UINT: return "FORMAT_R32G32B32_UINT";
		case DXGI_FORMAT_R32G32B32_SINT: return "FORMAT_R32G32B32_SINT";
		case DXGI_FORMAT_R16G16B16A16_TYPELESS: return "FORMAT_R16G16B16A16_TYPELESS";
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return "FORMAT_R16G16B16A16_FLOAT";
		case DXGI_FORMAT_R16G16B16A16_UNORM: return "FORMAT_R16G16B16A16_UNORM";
		case DXGI_FORMAT_R16G16B16A16_UINT: return "FORMAT_R16G16B16A16_UINT";
		case DXGI_FORMAT_R16G16B16A16_SNORM: return "FORMAT_R16G16B16A16_SNORM";
		case DXGI_FORMAT_R16G16B16A16_SINT: return "FORMAT_R16G16B16A16_SINT";
		case DXGI_FORMAT_R32G32_TYPELESS: return "FORMAT_R32G32_TYPELESS";
		case DXGI_FORMAT_R32G32_FLOAT: return "FORMAT_R32G32_FLOAT";
		case DXGI_FORMAT_R32G32_UINT: return "FORMAT_R32G32_UINT";
		case DXGI_FORMAT_R32G32_SINT: return "FORMAT_R32G32_SINT";
		case DXGI_FORMAT_R32G8X24_TYPELESS: return "FORMAT_R32G8X24_TYPELESS";
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return "FORMAT_D32_FLOAT_S8X24_UINT";
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return "FORMAT_R32_FLOAT_X8X24_TYPELESS";
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return "FORMAT_X32_TYPELESS_G8X24_UINT";
		case DXGI_FORMAT_R10G10B10A2_TYPELESS: return "FORMAT_R10G10B10A2_TYPELESS";
		case DXGI_FORMAT_R10G10B10A2_UNORM: return "FORMAT_R10G10B10A2_UNORM";
		case DXGI_FORMAT_R10G10B10A2_UINT: return "FORMAT_R10G10B10A2_UINT";
		case DXGI_FORMAT_R11G11B10_FLOAT: return "FORMAT_R11G11B10_FLOAT";
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: return "FORMAT_R8G8B8A8_TYPELESS";
		case DXGI_FORMAT_R8G8B8A8_UNORM: return "FORMAT_R8G8B8A8_UNORM";
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "FORMAT_R8G8B8A8_UNORM_SRGB";
		case DXGI_FORMAT_R8G8B8A8_UINT: return "FORMAT_R8G8B8A8_UINT";
		case DXGI_FORMAT_R8G8B8A8_SNORM: return "FORMAT_R8G8B8A8_SNORM";
		case DXGI_FORMAT_R8G8B8A8_SINT: return "FORMAT_R8G8B8A8_SINT";
		case DXGI_FORMAT_R16G16_TYPELESS: return "FORMAT_R16G16_TYPELESS";
		case DXGI_FORMAT_R16G16_FLOAT: return "FORMAT_R16G16_FLOAT";
		case DXGI_FORMAT_R16G16_UNORM: return "FORMAT_R16G16_UNORM";
		case DXGI_FORMAT_R16G16_UINT: return "FORMAT_R16G16_UINT";
		case DXGI_FORMAT_R16G16_SNORM: return "FORMAT_R16G16_SNORM";
		case DXGI_FORMAT_R16G16_SINT: return "FORMAT_R16G16_SINT";
		case DXGI_FORMAT_R32_TYPELESS: return "FORMAT_R32_TYPELESS";
		case DXGI_FORMAT_D32_FLOAT: return "FORMAT_D32_FLOAT";
		case DXGI_FORMAT_R32_FLOAT: return "FORMAT_R32_FLOAT";
		case DXGI_FORMAT_R32_UINT: return "FORMAT_R32_UINT";
		case DXGI_FORMAT_R32_SINT: return "FORMAT_R32_SINT";
		case DXGI_FORMAT_R24G8_TYPELESS: return "FORMAT_R24G8_TYPELESS";
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return "FORMAT_D24_UNORM_S8_UINT";
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return "FORMAT_R24_UNORM_X8_TYPELESS";
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return "FORMAT_X24_TYPELESS_G8_UINT";
		case DXGI_FORMAT_R8G8_TYPELESS: return "FORMAT_R8G8_TYPELESS";
		case DXGI_FORMAT_R8G8_UNORM: return "FORMAT_R8G8_UNORM";
		case DXGI_FORMAT_R8G8_UINT: return "FORMAT_R8G8_UINT";
		case DXGI_FORMAT_R8G8_SNORM: return "FORMAT_R8G8_SNORM";
		case DXGI_FORMAT_R8G8_SINT: return "FORMAT_R8G8_SINT";
		case DXGI_FORMAT_R16_TYPELESS: return "FORMAT_R16_TYPELESS";
		case DXGI_FORMAT_R16_FLOAT: return "FORMAT_R16_FLOAT";
		case DXGI_FORMAT_D16_UNORM: return "FORMAT_D16_UNORM";
		case DXGI_FORMAT_R16_UNORM: return "FORMAT_R16_UNORM";
		case DXGI_FORMAT_R16_UINT: return "FORMAT_R16_UINT";
		case DXGI_FORMAT_R16_SNORM: return "FORMAT_R16_SNORM";
		case DXGI_FORMAT_R16_SINT: return "FORMAT_R16_SINT";
		case DXGI_FORMAT_R8_TYPELESS: return "FORMAT_R8_TYPELESS";
		case DXGI_FORMAT_R8_UNORM: return "FORMAT_R8_UNORM";
		case DXGI_FORMAT_R8_UINT: return "FORMAT_R8_UINT";
		case DXGI_FORMAT_R8_SNORM: return "FORMAT_R8_SNORM";
		case DXGI_FORMAT_R8_SINT: return "FORMAT_R8_SINT";
		case DXGI_FORMAT_A8_UNORM: return "FORMAT_A8_UNORM";
		case DXGI_FORMAT_R1_UNORM: return "FORMAT_R1_UNORM";
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return "FORMAT_R9G9B9E5_SHAREDEXP";
		case DXGI_FORMAT_R8G8_B8G8_UNORM: return "FORMAT_R8G8_B8G8_UNORM";
		case DXGI_FORMAT_G8R8_G8B8_UNORM: return "FORMAT_G8R8_G8B8_UNORM";
		case DXGI_FORMAT_BC1_TYPELESS: return "FORMAT_BC1_TYPELESS";
		case DXGI_FORMAT_BC1_UNORM: return "FORMAT_BC1_UNORM";
		case DXGI_FORMAT_BC1_UNORM_SRGB: return "FORMAT_BC1_UNORM_SRGB";
		case DXGI_FORMAT_BC2_TYPELESS: return "FORMAT_BC2_TYPELESS";
		case DXGI_FORMAT_BC2_UNORM: return "FORMAT_BC2_UNORM";
		case DXGI_FORMAT_BC2_UNORM_SRGB: return "FORMAT_BC2_UNORM_SRGB";
		case DXGI_FORMAT_BC3_TYPELESS: return "FORMAT_BC3_TYPELESS";
		case DXGI_FORMAT_BC3_UNORM: return "FORMAT_BC3_UNORM";
		case DXGI_FORMAT_BC3_UNORM_SRGB: return "FORMAT_BC3_UNORM_SRGB";
		case DXGI_FORMAT_BC4_TYPELESS: return "FORMAT_BC4_TYPELESS";
		case DXGI_FORMAT_BC4_UNORM: return "FORMAT_BC4_UNORM";
		case DXGI_FORMAT_BC4_SNORM: return "FORMAT_BC4_SNORM";
		case DXGI_FORMAT_BC5_TYPELESS: return "FORMAT_BC5_TYPELESS";
		case DXGI_FORMAT_BC5_UNORM: return "FORMAT_BC5_UNORM";
		case DXGI_FORMAT_BC5_SNORM: return "FORMAT_BC5_SNORM";
		case DXGI_FORMAT_B5G6R5_UNORM: return "FORMAT_B5G6R5_UNORM";
		case DXGI_FORMAT_B5G5R5A1_UNORM: return "FORMAT_B5G5R5A1_UNORM";
		case DXGI_FORMAT_B8G8R8A8_UNORM: return "FORMAT_B8G8R8A8_UNORM";
		case DXGI_FORMAT_B8G8R8X8_UNORM: return "FORMAT_B8G8R8X8_UNORM";
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return "FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
		case DXGI_FORMAT_B8G8R8A8_TYPELESS: return "FORMAT_B8G8R8A8_TYPELESS";
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return "FORMAT_B8G8R8A8_UNORM_SRGB";
		case DXGI_FORMAT_B8G8R8X8_TYPELESS: return "FORMAT_B8G8R8X8_TYPELESS";
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return "FORMAT_B8G8R8X8_UNORM_SRGB";
		case DXGI_FORMAT_BC6H_TYPELESS: return "FORMAT_BC6H_TYPELESS";
		case DXGI_FORMAT_BC6H_UF16: return "FORMAT_BC6H_UF16";
		case DXGI_FORMAT_BC6H_SF16: return "FORMAT_BC6H_SF16";
		case DXGI_FORMAT_BC7_TYPELESS: return "FORMAT_BC7_TYPELESS";
		case DXGI_FORMAT_BC7_UNORM: return "FORMAT_BC7_UNORM";
		case DXGI_FORMAT_BC7_UNORM_SRGB: return "FORMAT_BC7_UNORM_SRGB";
		case DXGI_FORMAT_AYUV: return "FORMAT_AYUV";
		case DXGI_FORMAT_Y410: return "FORMAT_Y410";
		case DXGI_FORMAT_Y416: return "FORMAT_Y416";
		case DXGI_FORMAT_NV12: return "FORMAT_NV12";
		case DXGI_FORMAT_P010: return "FORMAT_P010";
		case DXGI_FORMAT_P016: return "FORMAT_P016";
		case DXGI_FORMAT_420_OPAQUE: return "FORMAT_420_OPAQUE";
		case DXGI_FORMAT_YUY2: return "FORMAT_YUY2";
		case DXGI_FORMAT_Y210: return "FORMAT_Y210";
		case DXGI_FORMAT_Y216: return "FORMAT_Y216";
		case DXGI_FORMAT_NV11: return "FORMAT_NV11";
		case DXGI_FORMAT_AI44: return "FORMAT_AI44";
		case DXGI_FORMAT_IA44: return "FORMAT_IA44";
		case DXGI_FORMAT_P8: return "FORMAT_P8";
		case DXGI_FORMAT_A8P8: return "FORMAT_A8P8";
		case DXGI_FORMAT_B4G4R4A4_UNORM: return "FORMAT_B4G4R4A4_UNORM";
		case DXGI_FORMAT_P208: return "FORMAT_P208";
		case DXGI_FORMAT_V208: return "FORMAT_V208";
		case DXGI_FORMAT_V408: return "FORMAT_V408";
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE: return "FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE";
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE: return "FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE";
		case DXGI_FORMAT_FORCE_UINT: return "FORMAT_FORCE_UINT";
		}
		return "Unknown !!";
	}

	const char*  getNameOfD3d12TextureLayout(D3D12_TEXTURE_LAYOUT layout)
	{
		switch (layout) {
		case D3D12_TEXTURE_LAYOUT_UNKNOWN: return "LAYOUT_UNKNOWN";
		case D3D12_TEXTURE_LAYOUT_ROW_MAJOR: return "LAYOUT_ROW_MAJOR";
		case D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE: return "LAYOUT_64KB_UNDEFINED_SWIZZLE";
		case D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE: return "LAYOUT_64KB_STANDARD_SWIZZLE";
		}
		return "Unknown !!";
	}
#endif // #if defined(_WIN32) || defined(_WIN64)
}
#pragma warning(pop)
