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

#include <unordered_map>

namespace std
{
	using tr1::unordered_map;
}

namespace micro_profiler
{
#pragma pack(push, 4)
	struct call_record
	{
		__int64 timestamp;
		const void *callee;	// call address + sizeof(void *) + 1 bytes
	};
#pragma pack(pop)

	struct address_compare;
	struct function_statistics;
	struct function_statistics_detailed;

	typedef std::unordered_map<const void * /*address*/, function_statistics, address_compare> statistics_map;
	typedef std::unordered_map<const void * /*address*/, unsigned __int64, address_compare> statistics_map_callers;
	typedef std::unordered_map<const void * /*address*/, function_statistics_detailed, address_compare> statistics_map_detailed;

	struct address_compare
	{
		size_t operator ()(const void *key) const throw();
	};

	struct function_statistics
	{
		explicit function_statistics(unsigned __int64 times_called = 0, unsigned __int64 max_reentrance = 0, __int64 inclusive_time = 0, __int64 exclusive_time = 0, __int64 max_call_time = 0);

		void add_call(unsigned __int64 level, __int64 inclusive_time, __int64 exclusive_time);

		unsigned __int64 times_called;
		unsigned __int64 max_reentrance;
		__int64 inclusive_time;
		__int64 exclusive_time;
		__int64 max_call_time;
	};

	struct function_statistics_detailed : function_statistics
	{
		statistics_map callees;
		statistics_map_callers callers;
	};


	// address_compare - inline definitions
	inline size_t address_compare::operator ()(const void *key) const throw()
	{	return (reinterpret_cast<size_t>(key) >> 4) * 2654435761;	}


	// function_statistics - inline definitions
	inline function_statistics::function_statistics(unsigned __int64 times_called_, unsigned __int64 max_reentrance_, __int64 inclusive_time_, __int64 exclusive_time_, __int64 max_call_time_)
		: times_called(times_called_), max_reentrance(max_reentrance_), inclusive_time(inclusive_time_), exclusive_time(exclusive_time_), max_call_time(max_call_time_)
	{	}

	inline void function_statistics::add_call(unsigned __int64 level, __int64 inclusive_time, __int64 exclusive_time)
	{
		++this->times_called;
		if (level > this->max_reentrance)
			this->max_reentrance = level;
		if (!level)
			this->inclusive_time += inclusive_time;
		this->exclusive_time += exclusive_time;
		if (inclusive_time > this->max_call_time)
			this->max_call_time = inclusive_time;
	}


	// helper methods - inline definitions
	inline void add_child_statistics(function_statistics &/*s*/, const void * /*function*/, unsigned __int64 /*level*/, __int64 /*inclusive_time*/, __int64 /*exclusive_time*/)
	{	}

	inline void add_child_statistics(function_statistics_detailed &s, const void *function, unsigned __int64 level, __int64 inclusive_time, __int64 exclusive_time)
	{	s.callees[function].add_call(level, inclusive_time, exclusive_time);	}

	inline void update_parent_statistics(statistics_map_detailed &s, const void *address, const function_statistics_detailed &f)
	{
		for (statistics_map::const_iterator i = f.callees.begin(); i != f.callees.end(); ++i)
			s[i->first].callers[address] = i->second.times_called;
	}
}
