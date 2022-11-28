#include "debug_win.h"
#include <cstdio>
#include <crtdbg.h>
#include <windows.h>
#include <exception>

namespace Dbg {
	void assert(bool b)
	{
		_ASSERT(b);
	}

	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}

		const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X",
				static_cast<unsigned int>(result));
			return s_str;
		}

	private:
		HRESULT result;
	};

    // Helper utility converts D3D API failures into exceptions.
    void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
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