#pragma once
#include <windows.h>

namespace MiniEngineIf {
	void init(HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight);
	void end();
	void loadModel();
}
