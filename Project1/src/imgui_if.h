#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <vector>

namespace ImguiIf {
	enum class ParamType {
		kInt,
		kFloat,
	};

	void init(ID3D12Device* device, HWND hwnd);
	void shutdown();
	LRESULT handleInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void startFrame();
	void update();
	void render(ID3D12GraphicsCommandList* pCommandList);

	template <typename T>
	LRESULT printParams(ParamType type, const std::string& str, const std::vector<T*>& ptrs);
}
