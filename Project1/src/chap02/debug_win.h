#pragma once
#include <windows.h>

namespace Dbg {
	inline void assert(bool b);
	void ThrowIfFailed(HRESULT hr);
	void print(const char* format, ...);
	void printLastError();
}
