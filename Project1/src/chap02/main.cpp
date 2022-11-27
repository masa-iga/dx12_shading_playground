#include <windows.h>
#include "windowManager_win.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WinMgr::setup(hInstance, nCmdShow);

	WinMgr::main();

	WinMgr::teardown();

	return 0;
}