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

#include "system.h"

#include <wpl/mt/thread.h>
#include <list>

namespace micro_profiler
{
	struct call_record;

	struct calls_collector_i
	{
		struct acceptor;

		virtual ~calls_collector_i() throw()	{	}
		virtual void read_collected(acceptor &a) = 0;
		virtual __int64 profiler_latency() const throw() = 0;
	};

	struct calls_collector_i::acceptor
	{
		virtual void accept_calls(unsigned int threadid, const call_record *calls, size_t count) = 0;
	};

	class calls_collector : public calls_collector_i
	{
		class thread_trace_block;

		static calls_collector _instance;

		const size_t _trace_limit;
		__int64 _profiler_latency;
		wpl::mt::tls<thread_trace_block> _trace_pointers_tls;
		mutex _thread_blocks_mtx;
		std::list<thread_trace_block> _call_traces;

		thread_trace_block &get_current_thread_trace();
		thread_trace_block &construct_thread_trace();

	public:
		calls_collector(size_t trace_limit);
		virtual ~calls_collector() throw();

		static calls_collector *instance() throw();
		virtual void read_collected(acceptor &a);

		void track(call_record call) throw();
		void track(__int64 timestamp, const void *address) throw();

		size_t trace_limit() const throw();
		virtual __int64 profiler_latency() const throw();
	};
}
