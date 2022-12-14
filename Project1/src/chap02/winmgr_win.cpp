#include "winmgr_win.h"
#include <windows.h>
#include "debug_win.h"

#define ENABLE_MINI_ENGINE_WINDOW (0)

namespace {
	HRESULT createWindow(HWND* hwnd, HINSTANCE hInstance, WNDPROC wndproc, const wchar_t className[], const wchar_t windowName[]);
	LRESULT CALLBACK WindowProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProcMiniEngine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT handleKeyDownMain(WPARAM wParam, LPARAM lParam);

	bool s_breakLoop = false;
	HWND s_hwndMain = NULL;
	HWND s_hwndMiniEngine = NULL;
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

		BOOL ret = GetMessage(&msg, NULL, wMsgFilterMin, wMsgFilterMax);
		Dbg::assert_(ret != -1);

		if (ret > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

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

	HWND getHwndMain()
	{
		return s_hwndMain;
	}

	HWND getHwndMiniEngine()
	{
		return s_hwndMiniEngine;
	}
}

namespace {
	HRESULT createWindow(HWND* hwnd, HINSTANCE hInstance, WNDPROC wndproc, const wchar_t className[], const wchar_t windowName[])
	{
		// Register the window class
		WNDCLASS wc = { };
		wc.lpfnWndProc = wndproc;
		wc.hInstance = hInstance;
		wc.lpszClassName = className;

		RegisterClass(&wc);

		// create the window
		*hwnd = CreateWindowEx(
			0,
			className,
			windowName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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
		switch (uMsg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
			handleKeyDownMain(wParam, lParam);
			return 0;

		default:
			break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT WindowProcMiniEngine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	HRESULT handleKeyDownMain(WPARAM wParam, LPARAM lParam)
	{
		switch (wParam) {
		case VK_ESCAPE:
			s_breakLoop = true;
			return S_OK;

		default:
			break;
		}

		return S_OK;
	}
}
