#include "imgui_if.h"
#include <array>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <vector>
#include <wrl.h>
#include "../../import/imgui/imgui.h"
#include "../../import/imgui/backends/imgui_impl_win32.h"
#include "../../import/imgui/backends/imgui_impl_dx12.h"
#include "config.h"
#include "debug_win.h"

#pragma comment(lib, "imgui.lib")

using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
template LRESULT ImguiIf::printParams<int32_t>(ParamType type, const std::string& str, const std::vector<int32_t*>& ptrs);
template LRESULT ImguiIf::printParams<uint64_t>(ParamType type, const std::string& str, const std::vector<uint64_t*>& ptrs);
template LRESULT ImguiIf::printParams<float>(ParamType type, const std::string& str, const std::vector<float*>& ptrs);

namespace {
	struct Log {
		ImguiIf::ParamType m_type = ImguiIf::ParamType::kInt32;
		size_t m_num = 0;
		std::string m_str = "";
		std::array<const void*, 4> m_ptrs = { nullptr, nullptr, nullptr, nullptr };
	};

	constexpr int32_t kNumFramesInFlight = 3;
	ComPtr<ID3D12DescriptorHeap> s_descHeap = nullptr;
	std::vector<Log> s_logs;

	void setWindowPositionAndSize();
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
		ImGui::Begin("Rendering params");
		{
			setWindowPositionAndSize();

			for (const Log& log : s_logs)
			{
				if (log.m_type == ParamType::kInt32)
				{
					std::array<const int32_t*, 4> p = { reinterpret_cast<const int32_t*>(log.m_ptrs[0])
						, reinterpret_cast<const int32_t*>(log.m_ptrs[1])
						, reinterpret_cast<const int32_t*>(log.m_ptrs[2])
						, reinterpret_cast<const int32_t*>(log.m_ptrs[3])
					};

					switch (log.m_num) {
					case 1: ImGui::Text("%s: %d\n", log.m_str.c_str(), *p.at(0)); break;
					case 2: ImGui::Text("%s: (%d %d)\n", log.m_str.c_str(), *p.at(0), *p.at(1)); break;
					case 3: ImGui::Text("%s: (%d %d %d)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2)); break;
					case 4: ImGui::Text("%s: (%d %d %d %d)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2), *p.at(3)); break;
					default: break;
					}
				}
				else if (log.m_type == ParamType::kUint64)
				{
					std::array<const uint64_t*, 4> p = { reinterpret_cast<const uint64_t*>(log.m_ptrs[0])
						, reinterpret_cast<const uint64_t*>(log.m_ptrs[1])
						, reinterpret_cast<const uint64_t*>(log.m_ptrs[2])
						, reinterpret_cast<const uint64_t*>(log.m_ptrs[3])
					};
					switch (log.m_num) {
					case 1: ImGui::Text("%s: %zd\n", log.m_str.c_str(), *p.at(0)); break;
					case 2: ImGui::Text("%s: (%zd %zd)\n", log.m_str.c_str(), *p.at(0), *p.at(1)); break;
					case 3: ImGui::Text("%s: (%zd %zd %zd)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2)); break;
					case 4: ImGui::Text("%s: (%zd %zd %zd %zd)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2), *p.at(3)); break;
					default: break;
					}
				}
				else if (log.m_type == ParamType::kFloat)
				{
					std::array<const float*, 4> p = { reinterpret_cast<const float*>(log.m_ptrs[0])
						, reinterpret_cast<const float*>(log.m_ptrs[1])
						, reinterpret_cast<const float*>(log.m_ptrs[2])
						, reinterpret_cast<const float*>(log.m_ptrs[3])
					};

					switch (log.m_num) {
					case 1: ImGui::Text("%s: %.2f\n", log.m_str.c_str(), *p.at(0)); break;
					case 2: ImGui::Text("%s: (%.2f %.2f)\n", log.m_str.c_str(), *p.at(0), *p.at(1)); break;
					case 3: ImGui::Text("%s: (%.2f %.2f %.2f)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2)); break;
					case 4: ImGui::Text("%s: (%.2f %.2f %.2f %.2f)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2), *p.at(3)); break;
					default: break;
					}
				}
			}
		}
		ImGui::End();
	}

	void render(ID3D12GraphicsCommandList* list)
	{
		ImGui::Render();

		list->SetDescriptorHeaps(1, s_descHeap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list);
	}

	template<typename T>
	LRESULT printParams(ParamType type, const std::string& str, const std::vector<T*>& ptrs)
	{
		if (ptrs.size() == 0)
			return S_OK;

		if (ptrs.size() > 4)
			return E_FAIL;

		Log log = { };
		log.m_num = 0;

		for (int32_t i = 0; i < ptrs.size(); ++i)
		{
			if (ptrs.at(i) == nullptr)
				break;

			log.m_ptrs.at(i) = reinterpret_cast<const void*>(ptrs.at(i));
			log.m_num++;
		}

		if (log.m_num == 0)
			return S_OK;

		log.m_type = type;
		log.m_str = str;
		s_logs.emplace_back(log);

		return S_OK;
	}

}

namespace {
	void setWindowPositionAndSize()
	{
		constexpr float kWidth = 500.0f;
		constexpr float kHeight = 100.0f;

		constexpr ImVec2 winPos = { static_cast<float>(Config::kRenderTargetWidth) - kWidth, 0.0f };
		ImGui::SetWindowPos(winPos, ImGuiCond_::ImGuiCond_Once);

		constexpr ImVec2 winSize = { kWidth, kHeight };
		ImGui::SetWindowSize(winSize, ImGuiCond_::ImGuiCond_Once);
	}
}