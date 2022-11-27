#pragma once

namespace Dbg {
	void assert(bool b);
	void print(const char* format, ...);
	void printLastError();
}
