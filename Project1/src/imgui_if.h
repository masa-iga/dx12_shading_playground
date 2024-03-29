#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <vector>

namespace ImguiIf {
	enum class ParamType {
		kFrame,
		kGpuTime,
		kGeneral,
	};

	enum class VarType {
		kInt32,
		kUint64,
		kFloat,
	};

	void init(ID3D12Device* device, HWND hwnd);
	void shutdown();
	LRESULT handleInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void startFrame();
	void update();
	void render(ID3D12GraphicsCommandList* pCommandList);

	template <typename T>
	LRESULT printParams(VarType type, const std::string& str, const std::vector<const T*>& ptrs, ParamType kind = ParamType::kGeneral);
}
