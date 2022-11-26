#include <windows.h>
#include "windowManager_win.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WinMgr::main(hInstance, nCmdShow);

	return 0;
}