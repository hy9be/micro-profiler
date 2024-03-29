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

#pragma once

#include "./../_generated/frontend.h"
#include "../resources/resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <memory>

namespace std 
{
	using tr1::shared_ptr;
}

extern "C" CLSID CLSID_ProfilerFrontend;

namespace micro_profiler
{
	class ProfilerMainDialog;
	struct functions_list;
	struct symbol_resolver;

	class ATL_NO_VTABLE ProfilerFrontend : public IProfilerFrontend, public CComObjectRootEx<CComSingleThreadModel>,
		public CComCoClass<ProfilerFrontend, &CLSID_ProfilerFrontend>
	{
		std::shared_ptr<symbol_resolver> _symbols;
		std::shared_ptr<functions_list> _statistics;
		std::shared_ptr<ProfilerMainDialog> _dialog;

	public:
		ProfilerFrontend();
		~ProfilerFrontend();

		DECLARE_REGISTRY_RESOURCEID(IDR_PROFILERSINK)

		BEGIN_COM_MAP(ProfilerFrontend)
			COM_INTERFACE_ENTRY(IProfilerFrontend)
		END_COM_MAP()

		void FinalRelease();

		STDMETHODIMP Initialize(long process_id, long long ticks_resolution);
		STDMETHODIMP LoadImages(long count, ImageLoadInfo *images);
		STDMETHODIMP UpdateStatistics(long count, FunctionStatisticsDetailed *statistics);
		STDMETHODIMP UnloadImages(long count, long long *image_addresses);
	};
}

typedef micro_profiler::ProfilerFrontend _ProfilerFrontend;

OBJECT_ENTRY_AUTO(CLSID_ProfilerFrontend, _ProfilerFrontend);
