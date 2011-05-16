//	Copyright (C) 2011 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <crtdefs.h>

namespace micro_profiler
{
#pragma pack(push, 4)
	struct call_record
	{
		void *callee;	// call address + 5 bytes
		__int64 timestamp;
	};
#pragma pack(pop)

	struct function_statistics
	{
		function_statistics(unsigned __int64 times_called = 0, unsigned __int64 max_reentrance = 0, __int64 inclusive_time = 0, __int64 exclusive_time = 0);

		unsigned __int64 times_called;
		unsigned __int64 max_reentrance;
		__int64 inclusive_time;
		__int64 exclusive_time;

		void add_call(unsigned __int64 level, __int64 inclusive_time, __int64 exclusive_time);
		void add(unsigned __int64 times_called, unsigned __int64 max_reentrance, __int64 inclusive_time, __int64 exclusive_time);
	};

	struct address_compare
	{
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;

		size_t operator ()(void *key) const;
		bool operator ()(const void *lhs, const void *rhs) const;
	};


	inline function_statistics::function_statistics(unsigned __int64 times_called_, unsigned __int64 max_reentrance_, __int64 inclusive_time_, __int64 exclusive_time_)
		: times_called(times_called_), max_reentrance(max_reentrance_), inclusive_time(inclusive_time_), exclusive_time(exclusive_time_)
	{	}

	inline void function_statistics::add_call(unsigned __int64 level, __int64 inclusive_time, __int64 exclusive_time)
	{
		++this->times_called;
		if (level > this->max_reentrance)
			this->max_reentrance = level;
		if (!level)
			this->inclusive_time += inclusive_time;
		this->exclusive_time += exclusive_time;
	}

	inline void function_statistics::add(unsigned __int64 times_called, unsigned __int64 max_reentrance, __int64 inclusive_time, __int64 exclusive_time)
	{
		this->times_called += times_called;
		if (max_reentrance > this->max_reentrance)
			this->max_reentrance = max_reentrance;
		this->inclusive_time += inclusive_time;
		this->exclusive_time += exclusive_time;
	}


	inline size_t address_compare::operator ()(void *key) const
	{	return (reinterpret_cast<size_t>(key) >> 4) * 2654435761;	}

	inline bool address_compare::operator ()(const void *lhs, const void *rhs) const
	{	return lhs < rhs;	}
}