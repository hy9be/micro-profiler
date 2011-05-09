#pragma once

#include "./../_generated/microprofilerfrontend_i.h"
#include "./../data_structures.h"

#include <tchar.h>
#include <hash_map>
#include <string>
#include <vector>
#include <memory>

class symbol_resolver;

typedef std::basic_string<TCHAR> tstring;

struct function_statistics_ex : micro_profiler::function_statistics
{
	function_statistics_ex(const FunctionStatistics &from, const symbol_resolver &resolver);

	tstring name;
};

class statistics
{
	typedef stdext::hash_map<__int64, function_statistics_ex> statistics_map_;
	class dereferencing_wrapper;

	const symbol_resolver &_symbol_resolver;
	std::auto_ptr<dereferencing_wrapper> _predicate;
	statistics_map_ _statistics;
	std::vector<statistics_map_::const_iterator> _sorted_statistics;

	statistics(const statistics &);
	void operator =(const statistics &);

public:
	typedef statistics_map_ statistics_map;
	typedef bool (*sort_predicate)(const function_statistics_ex &lhs, const function_statistics_ex &rhs);

public:
	statistics(const symbol_resolver &resolver);
	virtual ~statistics();

	const function_statistics_ex &at(unsigned int index) const;
	unsigned int size() const;

	void clear();
	void sort(sort_predicate predicate, bool ascending);
	bool update(const FunctionStatistics *data, unsigned int count);
};
