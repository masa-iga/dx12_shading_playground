#include "winmgr_win.h"
#include <windows.h>
#include "debug_win.h"

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HRESULT handleKeyDown(WPARAM wParam, LPARAM lParam);

static bool s_breakLoop = false;
static HWND s_hwnd = 0;

namespace WinMgr {
	HRESULT setup(HINSTANCE hInstance, int nCmdShow)
	{
		// Register the window class
		constexpr wchar_t kClassName[] = L"Sample Window Class";
		constexpr wchar_t kWindowName[] = L"DX12 Shading Playground";

		WNDCLASS wc = { };
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = kClassName;

		RegisterClass(&wc);

		// create the window

		s_hwnd = CreateWindowEx(
			0,
			kClassName,
			kWindowName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (s_hwnd == NULL)
		{
			Dbg::printLastError();
			Dbg::assert_(s_hwnd != NULL);
			return S_FALSE;
		}

		ShowWindow(s_hwnd, nCmdShow);

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
		BOOL ret = DestroyWindow(s_hwnd);

		if (ret == false)
		{
			Dbg::printLastError();
			Dbg::assert_(ret);
		}

		return S_OK;
	}

	HWND getHwnd()
	{
		return s_hwnd;
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		handleKeyDown(wParam, lParam);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT handleKeyDown(WPARAM wParam, LPARAM lParam)
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
