#pragma once
#include <windows.h>
#include <exception>
#include <cstdio>
#include <D3Dcommon.h>

namespace Dbg {
	inline void assert_(bool b);
	void print(const char* format, ...);
	void printBlob(ID3DBlob* errorBlob);
	void printLastError();

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
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}
