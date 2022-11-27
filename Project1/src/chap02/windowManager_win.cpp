#include "windowManager_win.h"
#include <windows.h>
#include "debug_win.h"

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HRESULT handleKeyDown(WPARAM wParam, LPARAM lParam);

static bool s_messageLoop = true;

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

		HWND hwnd = CreateWindowEx(
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

		if (hwnd == NULL)
		{
			Dbg::printLastError();
			Dbg::assert(hwnd != NULL);
			return S_FALSE;
		}

		ShowWindow(hwnd, nCmdShow);

		return S_OK;
	}

	HRESULT main()
	{
		// Run the message loop
		MSG msg = { };

		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (!s_messageLoop)
				break;
		}

		return S_OK;
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
		s_messageLoop = false;
		return S_OK;

	default:
		break;
	}

	return S_OK;
}
