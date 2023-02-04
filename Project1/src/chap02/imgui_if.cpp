#include "imgui_if.h"
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include "../../import/imgui/imgui.h"
#include "../../import/imgui/backends/imgui_impl_win32.h"
#include "../../import/imgui/backends/imgui_impl_dx12.h"
#include "config.h"
#include "debug_win.h"

#pragma comment(lib, "imgui.lib")

using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	constexpr int32_t kNumFramesInFlight = 3;
	ComPtr<ID3D12DescriptorHeap> s_descHeap = nullptr;
}

namespace ImguiIf {
	void init(ID3D12Device* device, HWND hwnd)
	{
		IMGUI_CHECKVERSION();

		auto context = ImGui::CreateContext();
		Dbg::assert_(context != nullptr);

		// disable create ini file
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;

		ImGui::StyleColorsDark();

		bool bRet = ImGui_ImplWin32_Init(hwnd);
		Dbg::assert_(bRet);

		{
			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				.NodeMask = 0,
			};

			auto result = device->CreateDescriptorHeap(
				&heapDesc,
				IID_PPV_ARGS(s_descHeap.ReleaseAndGetAddressOf()));
			Dbg::ThrowIfFailed(result);
		}

		bRet = ImGui_ImplDX12_Init(
			device,
			kNumFramesInFlight,
			Config::kRenderTargetFormat,
			s_descHeap.Get(),
			s_descHeap->GetCPUDescriptorHandleForHeapStart(),
			s_descHeap->GetGPUDescriptorHandleForHeapStart());
		Dbg::assert_(bRet);
	}

	void shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	LRESULT handleInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}

	void startFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void update()
	{
		ImGui::Begin("Performance");
		{
			// set position and size
			{
				constexpr float kWidth = 500.0f;
				constexpr float kHeight = 300.0f;

				constexpr ImVec2 winPos = { static_cast<float>(Config::kRenderTargetWidth) - kWidth, 0.f };
				ImGui::SetWindowPos(winPos, ImGuiCond_::ImGuiCond_Once);

				constexpr ImVec2 winSize = { kWidth, kHeight };
				ImGui::SetWindowSize(winSize, ImGuiCond_::ImGuiCond_Once);
			}

			ImGui::Text("Hello world");

			if (ImGui::Button("Save"))
			{
				;
			}

			char buf[128];
			ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));

			float f = 0.0f;
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		}
		ImGui::End();
	}

	void render(ID3D12GraphicsCommandList* list)
	{
		ImGui::Render();

		list->SetDescriptorHeaps(1, s_descHeap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list);
	}
}