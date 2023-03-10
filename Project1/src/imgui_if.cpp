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
template LRESULT ImguiIf::printParams<int32_t>(VarType type, const std::string& str, const std::vector<int32_t*>& ptrs, ParamType kind);
template LRESULT ImguiIf::printParams<uint64_t>(VarType type, const std::string& str, const std::vector<uint64_t*>& ptrs, ParamType kind);
template LRESULT ImguiIf::printParams<float>(VarType type, const std::string& str, const std::vector<float*>& ptrs, ParamType kind);

namespace {
	struct Log {
		ImguiIf::VarType m_type = ImguiIf::VarType::kInt32;
		size_t m_num = 0;
		std::string m_str = "";
		std::array<const void*, 4> m_ptrs = { nullptr, nullptr, nullptr, nullptr };
	};

	const char winTitle[] = "Rendering params";
	constexpr int32_t kNumFramesInFlight = 3;
	ComPtr<ID3D12DescriptorHeap> s_descHeap = nullptr;
	std::vector<Log> s_logs;

	void setNextWindowPositionAndSize(ImVec2 winPos, ImVec2 winSize);
	bool isParamNumChanged();
	std::pair<float, float> computeWinPos();
	std::pair<float, float> computeWinSize();

	template<typename T>
	LRESULT printParamsFrame(ImguiIf::VarType type, const std::string& str, const std::vector<T*>& ptrs)
	{
		if (type != ImguiIf::VarType::kUint64)
			return E_FAIL;

		if (ptrs.size() != 1)
			return E_FAIL;

		if (ptrs.at(0) == nullptr)
			return S_OK;

		s_logs.at(static_cast<uint32_t>(ImguiIf::ParamType::kFrame)) = { type,  1, str, { ptrs.at(0), nullptr, nullptr, nullptr } };

		return S_OK;
	}

	template<typename T>
	LRESULT printParamsGpuTime(ImguiIf::VarType type, const std::string& str, const std::vector<T*>& ptrs)
	{
		if (type != ImguiIf::VarType::kFloat)
			return E_FAIL;

		if (ptrs.size() != 1)
			return E_FAIL;

		if (ptrs.at(0) == nullptr)
			return S_OK;

		s_logs.at(static_cast<uint32_t>(ImguiIf::ParamType::kGpuTime)) = { type,  1, str, { ptrs.at(0), nullptr, nullptr, nullptr } };

		return S_OK;
	}

	template<typename T>
	LRESULT printParamsGeneral(ImguiIf::VarType type, const std::string& str, const std::vector<T*>& ptrs)
	{
		if (ptrs.size() == 0)
			return S_OK;

		if (ptrs.size() > 4)
			return E_FAIL;

		Log log = {
			.m_type = type,
			.m_num = 0,
			.m_str = str,
			.m_ptrs = { nullptr, nullptr, nullptr, nullptr },
		};

		for (int32_t i = 0; i < ptrs.size(); ++i)
		{
			if (ptrs.at(i) == nullptr)
				break;

			log.m_ptrs.at(i) = reinterpret_cast<const void*>(ptrs.at(i));
			log.m_num++;
		}

		if (log.m_num == 0)
			return S_OK;

		s_logs.emplace_back(log);

		return S_OK;
	}
}

namespace ImguiIf {
	void init(ID3D12Device* device, HWND hwnd)
	{
		s_logs.resize(2);
		{
			s_logs.at(0) = { ImguiIf::VarType::kUint64, 0, "Frame", { nullptr, nullptr, nullptr, nullptr } };
			s_logs.at(1) = { ImguiIf::VarType::kUint64, 0, "GpuTime", { nullptr, nullptr, nullptr, nullptr } };
		}

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
		if (isParamNumChanged())
		{
			const ImVec2 winPos = { computeWinPos().first,  computeWinPos().second };
			const ImVec2 winSize = { computeWinSize().first,  computeWinSize().second };
			setNextWindowPositionAndSize(winPos, winSize);
		}

		ImGui::Begin(winTitle);
		{
			for (const Log& log : s_logs)
			{
				if (log.m_type == VarType::kInt32)
				{
					std::array<const int32_t*, 4> p = { reinterpret_cast<const int32_t*>(log.m_ptrs[0])
						, reinterpret_cast<const int32_t*>(log.m_ptrs[1])
						, reinterpret_cast<const int32_t*>(log.m_ptrs[2])
						, reinterpret_cast<const int32_t*>(log.m_ptrs[3])
					};

					switch (log.m_num) {
					case 1: ImGui::Text("%-13s: %d\n", log.m_str.c_str(), *p.at(0)); break;
					case 2: ImGui::Text("%-13s: (%d %d)\n", log.m_str.c_str(), *p.at(0), *p.at(1)); break;
					case 3: ImGui::Text("%-13s: (%d %d %d)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2)); break;
					case 4: ImGui::Text("%-13s: (%d %d %d %d)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2), *p.at(3)); break;
					default: break;
					}
				}
				else if (log.m_type == VarType::kUint64)
				{
					std::array<const uint64_t*, 4> p = { reinterpret_cast<const uint64_t*>(log.m_ptrs[0])
						, reinterpret_cast<const uint64_t*>(log.m_ptrs[1])
						, reinterpret_cast<const uint64_t*>(log.m_ptrs[2])
						, reinterpret_cast<const uint64_t*>(log.m_ptrs[3])
					};
					switch (log.m_num) {
					case 1: ImGui::Text("%-13s: %zd\n", log.m_str.c_str(), *p.at(0)); break;
					case 2: ImGui::Text("%-13s: (%zd %zd)\n", log.m_str.c_str(), *p.at(0), *p.at(1)); break;
					case 3: ImGui::Text("%-13s: (%zd %zd %zd)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2)); break;
					case 4: ImGui::Text("%-13s: (%zd %zd %zd %zd)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2), *p.at(3)); break;
					default: break;
					}
				}
				else if (log.m_type == VarType::kFloat)
				{
					std::array<const float*, 4> p = { reinterpret_cast<const float*>(log.m_ptrs[0])
						, reinterpret_cast<const float*>(log.m_ptrs[1])
						, reinterpret_cast<const float*>(log.m_ptrs[2])
						, reinterpret_cast<const float*>(log.m_ptrs[3])
					};

					switch (log.m_num) {
					case 1: ImGui::Text("%-13s: %.2f\n", log.m_str.c_str(), *p.at(0)); break;
					case 2: ImGui::Text("%-13s: (%.2f %.2f)\n", log.m_str.c_str(), *p.at(0), *p.at(1)); break;
					case 3: ImGui::Text("%-13s: (%.2f %.2f %.2f)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2)); break;
					case 4: ImGui::Text("%-13s: (%.2f %.2f %.2f %.2f)\n", log.m_str.c_str(), *p.at(0), *p.at(1), *p.at(2), *p.at(3)); break;
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
	LRESULT printParams(VarType type, const std::string& str, const std::vector<T*>& ptrs, ParamType kind)
	{
		switch (kind) {
		case ParamType::kFrame: return printParamsFrame<T>(type, str, ptrs);
		case ParamType::kGpuTime: return printParamsGpuTime<T>(type, str, ptrs);
		case ParamType::kGeneral: return printParamsGeneral<T>(type, str, ptrs);
		default: break;
		}

		return S_OK;
	}

}

namespace {
	void setNextWindowPositionAndSize(ImVec2 winPos, ImVec2 winSize)
	{
		ImGui::SetNextWindowSize(winSize, ImGuiCond_::ImGuiCond_Once);
		ImGui::SetNextWindowPos(winPos, ImGuiCond_::ImGuiCond_Once);
	}

	bool isParamNumChanged()
	{
		static size_t s_prevSize = 0;
		const size_t curSize = s_logs.size();

		if (curSize == s_prevSize)
			return false;

		s_prevSize = curSize;
		return true;
	}

	std::pair<float, float> computeWinPos()
	{
		return std::pair<float, float>(0.0f, 0.0f); // left-upper corner
	}

	std::pair<float, float> computeWinSize()
	{
		return std::pair<float, float>(0.0f, 0.0f); // IMGUI will automatically computes size
	}
}