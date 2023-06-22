#include "winmgr_win.h"
#include <vector>
#include <windows.h>
#include "config.h"
#include "debug_win.h"
#include "imgui_if.h"

#define ENABLE_MINI_ENGINE_WINDOW (0)

namespace {
	HRESULT createWindow(HWND* hwnd, HINSTANCE hInstance, WNDPROC wndproc, const wchar_t className[], const wchar_t windowName[]);
	LRESULT CALLBACK WindowProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProcMiniEngine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT handleKeyDownMain(WPARAM wParam, LPARAM lParam);
	void notify(WPARAM wParam, LPARAM lParam);

	bool s_breakLoop = false;
	HWND s_hwndMain = NULL;
	HWND s_hwndMiniEngine = NULL;
	std::vector<WinMgr::Iobserber*> m_obserbers;
}

namespace WinMgr {
	HRESULT setup(HINSTANCE hInstance, int nCmdShow)
	{
		Dbg::ThrowIfFailed(createWindow(&s_hwndMain, hInstance, WindowProcMain, L"Sample Window Class", L"DX12 Shading Playground"));
		Dbg::ThrowIfFailed(createWindow(&s_hwndMiniEngine, hInstance, WindowProcMiniEngine, L"Mini Engine Class", L"Mini Engine Window"));

		ShowWindow(s_hwndMain, nCmdShow);
#if ENABLE_MINI_ENGINE_WINDOW
		ShowWindow(s_hwndMiniEngine, nCmdShow);
#endif // #if ENABLE_MINI_ENGINE_WINDOW

		return S_OK;
	}

	bool handleMessage()
	{
		// Run the message loop
		MSG msg = { };
		const UINT wMsgFilterMin = 0;
		const UINT wMsgFilterMax = 0;
		const UINT wRemoveMsg = PM_REMOVE;

		BOOL ret = ::PeekMessage(&msg, NULL, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

		if (ret == 0)
			return s_breakLoop;

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);

		return s_breakLoop;
	}

	HRESULT teardown()
	{
		BOOL ret = DestroyWindow(s_hwndMain);

		if (ret == false)
		{
			Dbg::printLastError();
			Dbg::assert_(ret);
		}

		ret = DestroyWindow(s_hwndMiniEngine);

		if (ret == false)
		{
			Dbg::printLastError();
			Dbg::assert_(ret);
		}

		return S_OK;
	}

	HWND getHwnd(Handle handle)
	{
		switch (handle) {
		case Handle::kMain: return s_hwndMain;
		case Handle::kMiniEngine: return s_hwndMiniEngine;
		default: break;
		}

		return 0;
	}

	bool isWindowActive(Handle handle)
	{
		HWND ref = 0;

		switch (handle) {
		case Handle::kMain: ref = getHwnd(Handle::kMain); break;
		case Handle::kMiniEngine: ref = getHwnd(Handle::kMiniEngine); break;
		default: return false;
		}

		const HWND h = ::GetActiveWindow();

		return (h == ref);
	}

	bool addObserver(Iobserber* obserber)
	{
		m_obserbers.emplace_back(obserber);
		return true;
	}

	bool removeObserver(Iobserber* obserber)
	{
		for (size_t i = 0; i < m_obserbers.size(); ++i)
		{
			if (m_obserbers[i] == obserber)
			{
				//Dbg::print("L%d %s(): removed (%d 0x%p)\n", __LINE__, __func__, i, m_obserbers[i]);
				m_obserbers.erase(m_obserbers.begin() + i);
			}
		}

		return true;
	}
}

namespace {
	HRESULT createWindow(HWND* hwnd, HINSTANCE hInstance, WNDPROC wndproc, const wchar_t className[], const wchar_t windowName[])
	{
		// Register the window class
		WNDCLASS wc = { };
		{
			wc.lpfnWndProc = wndproc;
			wc.hInstance = hInstance;
			wc.lpszClassName = className;
		}

		::RegisterClass(&wc);

		::RECT wrc = { 0, 0, Config::kRenderTargetWidth, Config::kRenderTargetHeight };
		{
			auto b = ::AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
			Dbg::assert_(b == 1);
		}

		// create the window
		*hwnd = ::CreateWindowEx(
			0,
			className,
			windowName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			wrc.right - wrc.left,
			wrc.bottom - wrc.top,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (*hwnd == NULL)
		{
			Dbg::printLastError();
			Dbg::assert_(*hwnd != NULL);
			return S_FALSE;
		}

		return S_OK;
	}

	LRESULT CALLBACK WindowProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (ImguiIf::handleInput(hwnd, uMsg, wParam, lParam))
			return true;

		switch (uMsg) {
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
			handleKeyDownMain(wParam, lParam);
			return 0;

		default:
			break;
		}

		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT WindowProcMiniEngine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	HRESULT handleKeyDownMain(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
	{
		switch (wParam) {
		case VK_ESCAPE:
			s_breakLoop = true;
			return S_OK;

		default:
			notify(wParam, lParam);
			break;
		}

		return S_OK;
	}

	void notify(WPARAM wParam, LPARAM lParam)
	{
		for (auto& o : m_obserbers)
		{
			o->update(wParam, lParam);
		}
	}
}
