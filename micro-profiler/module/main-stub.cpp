//	Copyright (c) 2011-2014 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include "../entry.h"

#include <windows.h>

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE /*hinstance*/, DWORD /*reason*/, LPVOID /*reserved*/)
{
	return TRUE;
}

#ifdef _M_IX86
extern "C" __declspec(naked) void profile_enter()
{	__asm ret	}

extern "C" __declspec(naked) void profile_exit()
{	__asm ret	}
#else
extern "C" void profile_enter()
{	}

extern "C" void profile_exit()
{	}
#endif

extern "C" micro_profiler::handle *micro_profiler_initialize(const void * /*image_load_address*/)
{
	return 0;
}

STDAPI DllCanUnloadNow()
{	return S_OK;	}

STDAPI DllGetClassObject(REFCLSID /*rclsid*/, REFIID /*riid*/, LPVOID * /*ppv*/)
{	return S_OK;	}

STDAPI DllRegisterServer()
{	return S_OK;	}

STDAPI DllUnregisterServer()
{	return S_OK;	}
