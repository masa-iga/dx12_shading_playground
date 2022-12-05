#include "debug_win.h"
#include <cstdio>
#include <crtdbg.h>
#include <windows.h>
#include <string>

namespace Dbg {
	void assert(bool b)
	{
		_ASSERT(b);
	}

	void print(const char* format, ...)
	{
		va_list valist;
		va_start(valist, format);
		char tmp[256];
		auto ret = vsprintf_s(tmp, format, valist);
		Dbg::assert(ret >= 0);
		OutputDebugStringA(tmp);
		va_end(valist);
	}

	void printBlob(ID3DBlob* errorBlob)
	{
		if (errorBlob == nullptr)
			return;

		std::string errStr;
		errStr.resize(errorBlob->GetBufferSize());

		std::copy_n(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize(), errStr.begin());

		print("%s\n", errStr.c_str());
	}

	void printLastError()
	{
		const DWORD dw = GetLastError();
		LPWSTR lpMsgBuf = NULL;

		DWORD ret = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL /* lpSource */,
			dw /* dwMessageId */,
			0 /* dwLanguageId */,
			(LPWSTR)&lpMsgBuf /* lpBuffer */,
			0 /* nSize */,
			NULL /* Arguments */
		);

		if (ret == 0)
		{
			Dbg::assert(ret != 0);
			return;
		}

		constexpr size_t kLen = 256;
		TCHAR message[kLen];

		wsprintf(message, L"[Debug] errCode = %d: %s", dw, lpMsgBuf);

		OutputDebugStringW(message);
	}
}