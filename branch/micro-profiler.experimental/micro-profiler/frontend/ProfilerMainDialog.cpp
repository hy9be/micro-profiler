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

#include "ProfilerMainDialog.h"

#include "symbol_resolver.h"

#include <sstream>
#include <algorithm>
#include <math.h>

extern HINSTANCE g_instance;

using namespace std;

namespace micro_profiler
{
	namespace
	{
		__int64 g_ticks_resolution(1);

		template <typename T>
		tstring to_string(const T &value)
		{
			basic_stringstream<TCHAR> s;

			s.precision(3);
			s << value;
			return s.str();
		}

		tstring print_time(double value)
		{
			if (0.000001 > fabs(value))
				return to_string(1000000000 * value) + _T("ns");
			else if (0.001 > fabs(value))
				return to_string(1000000 * value) + _T("us");
			else if (1 > fabs(value))
				return to_string(1000 * value) + _T("ms");
			else
				return to_string(value) + _T("s");
		}

		tstring print_name(const statistics::statistics_entry &s, const symbol_resolver &resolver)
		{	return resolver.symbol_name_by_va(s.first);	}

		tstring print_times_called(const statistics::statistics_entry &s, const symbol_resolver &/*resolver*/)
		{	return to_string(s.second.times_called);	}

		tstring print_exclusive_time(const statistics::statistics_entry &s, const symbol_resolver &/*resolver*/)
		{	return print_time(1.0 * s.second.exclusive_time / g_ticks_resolution);	}

		tstring print_inclusive_time(const statistics::statistics_entry &s, const symbol_resolver &/*resolver*/)
		{	return print_time(1.0 * s.second.inclusive_time / g_ticks_resolution);	}

		tstring print_avg_exclusive_call_time(const statistics::statistics_entry &s, const symbol_resolver &/*resolver*/)
		{	return print_time(s.second.times_called ? 1.0 * s.second.exclusive_time / g_ticks_resolution / s.second.times_called : 0);	}

		tstring print_avg_inclusive_call_time(const statistics::statistics_entry &s, const symbol_resolver &/*resolver*/)
		{	return print_time(s.second.times_called ? 1.0 * s.second.inclusive_time / g_ticks_resolution / s.second.times_called : 0);	}

		tstring print_max_reentrance(const statistics::statistics_entry &s, const symbol_resolver &/*resolver*/)
		{	return to_string(s.second.max_reentrance);	}

		bool sort_by_name(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &resolver)
		{	return resolver.symbol_name_by_va(lhs.first) < resolver.symbol_name_by_va(rhs.first);	}

		bool sort_by_times_called(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &/*resolver*/)
		{	return lhs.second.times_called < rhs.second.times_called;	}

		bool sort_by_exclusive_time(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &/*resolver*/)
		{	return lhs.second.exclusive_time < rhs.second.exclusive_time;	}

		bool sort_by_inclusive_time(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &/*resolver*/)
		{	return lhs.second.inclusive_time < rhs.second.inclusive_time;	}

		bool sort_by_avg_exclusive_call_time(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &/*resolver*/)
		{	return (lhs.second.times_called ? lhs.second.exclusive_time / lhs.second.times_called : 0) < (rhs.second.times_called ? rhs.second.exclusive_time / rhs.second.times_called : 0);	}

		bool sort_by_avg_inclusive_call_time(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &/*resolver*/)
		{	return (lhs.second.times_called ? lhs.second.inclusive_time / lhs.second.times_called : 0) < (rhs.second.times_called ? rhs.second.inclusive_time / rhs.second.times_called : 0);	}

		bool sort_by_max_reentrance(const statistics::statistics_entry &lhs, const statistics::statistics_entry &rhs, const symbol_resolver &/*resolver*/)
		{	return lhs.second.max_reentrance < rhs.second.max_reentrance;	}
	}

	ProfilerMainDialog::ProfilerMainDialog(statistics &s, const symbol_resolver &resolver, __int64 ticks_resolution)
		: _statistics(s), _resolver(resolver), _last_sort_column(-1), _last_children_sort_column(-1), _last_selected(0)
	{
		g_ticks_resolution = ticks_resolution;

		_printers[0] = &print_name;
		_printers[1] = &print_times_called;
		_printers[2] = &print_exclusive_time;
		_printers[3] = &print_inclusive_time;
		_printers[4] = &print_avg_exclusive_call_time;
		_printers[5] = &print_avg_inclusive_call_time;
		_printers[6] = &print_max_reentrance;

		_sorters[0] = make_pair(&sort_by_name, true);
		_sorters[1] = make_pair(&sort_by_times_called, false);
		_sorters[2] = make_pair(&sort_by_exclusive_time, false);
		_sorters[3] = make_pair(&sort_by_inclusive_time, false);
		_sorters[4] = make_pair(&sort_by_avg_exclusive_call_time, false);
		_sorters[5] = make_pair(&sort_by_avg_inclusive_call_time, false);
		_sorters[6] = make_pair(&sort_by_max_reentrance, false);

		Create(NULL, 0);

		LVCOLUMN columns[] = {
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 60, _T("Function"), 0, 0, 0, 0,	},
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 70, _T("Times Called"), 0, 1, 0, 1,	},
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 70, _T("Exclusive Time"), 0, 2, 0, 2,	},
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 70, _T("Inclusive Time"), 0, 3, 0, 3,	},
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 70, _T("Average Call Time (Exclusive)"), 0, 4, 0, 4,	},
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 70, _T("Average Call Time (Inclusive)"), 0, 5, 0, 5,	},
			{ LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_WIDTH, 0, 70, _T("Max Recursion"), 0, 6, 0, 6,	},
		};

		ListView_InsertColumn(_statistics_view, 0, &columns[0]);
		ListView_InsertColumn(_statistics_view, 1, &columns[1]);
		ListView_InsertColumn(_statistics_view, 2, &columns[2]);
		ListView_InsertColumn(_statistics_view, 3, &columns[3]);
		ListView_InsertColumn(_statistics_view, 4, &columns[4]);
		ListView_InsertColumn(_statistics_view, 5, &columns[5]);
		ListView_InsertColumn(_statistics_view, 6, &columns[6]);
		ListView_SetExtendedListViewStyle(_statistics_view, LVS_EX_FULLROWSELECT | ListView_GetExtendedListViewStyle(_statistics_view));

		ListView_InsertColumn(_children_statistics_view, 0, &columns[0]);
		ListView_InsertColumn(_children_statistics_view, 1, &columns[1]);
		ListView_InsertColumn(_children_statistics_view, 2, &columns[2]);
		ListView_InsertColumn(_children_statistics_view, 3, &columns[3]);
		ListView_InsertColumn(_children_statistics_view, 4, &columns[4]);
		ListView_InsertColumn(_children_statistics_view, 5, &columns[5]);
		ListView_InsertColumn(_children_statistics_view, 6, &columns[6]);
		ListView_SetExtendedListViewStyle(_children_statistics_view, LVS_EX_FULLROWSELECT | ListView_GetExtendedListViewStyle(_children_statistics_view));
	}

	ProfilerMainDialog::~ProfilerMainDialog()
	{
		if (IsWindow())
			DestroyWindow();
	}

	void ProfilerMainDialog::RefreshList(unsigned int new_count)
	{
		ListView_SetItemCountEx(_statistics_view, new_count, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		ListView_SetItemCountEx(_children_statistics_view, _statistics.size_children(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		_statistics_view.Invalidate(FALSE);
		_children_statistics_view.Invalidate(FALSE);
		SelectByAddress(_last_selected);
	}

	LRESULT ProfilerMainDialog::OnInitDialog(UINT /*message*/, WPARAM /*wparam*/, LPARAM /*lparam*/, BOOL& handled)
	{
		CRect clientRect;

		_statistics_view = GetDlgItem(IDC_FUNCTIONS_STATISTICS);
		_children_statistics_view = GetDlgItem(IDC_CHILDREN_STATISTICS);
		_clear_button = GetDlgItem(IDC_BTN_CLEAR);
		_copy_all_button = GetDlgItem(IDC_BTN_COPY_ALL);

		GetClientRect(&clientRect);
		RelocateControls(clientRect.Size());

		::EnableMenuItem(GetSystemMenu(FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		SetIcon(::LoadIcon(g_instance, MAKEINTRESOURCE(IDI_APPMAIN)), TRUE);
		handled = TRUE;
		return 1;  // Let the system set the focus
	}

	LRESULT ProfilerMainDialog::OnSize(UINT /*message*/, WPARAM /*wparam*/, LPARAM lparam, BOOL &handled)
	{
		RelocateControls(CSize(LOWORD(lparam), HIWORD(lparam)));
		handled = TRUE;
		return 1;
	}

	LRESULT ProfilerMainDialog::OnGetDispInfo(int control_id, LPNMHDR pnmh, BOOL &handled)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;

		if (LVIF_TEXT & pdi->item.mask)
		{
			tstring item_text(_printers[pdi->item.iSubItem](control_id == IDC_CHILDREN_STATISTICS ? _statistics.at_children(pdi->item.iItem) : _statistics.at(pdi->item.iItem), _resolver));
		
			_tcsncpy_s(pdi->item.pszText, pdi->item.cchTextMax, item_text.c_str(), _TRUNCATE);
			handled = TRUE;
		}
		return 0;
	}

	LRESULT ProfilerMainDialog::OnColumnSort(int control_id, LPNMHDR pnmh, BOOL &handled)
	{
		CWindow &view = IDC_CHILDREN_STATISTICS == control_id ? _children_statistics_view : _statistics_view;
		bool &sort_ascending = IDC_CHILDREN_STATISTICS == control_id ? _sort_children_ascending : _sort_ascending;
		int &last_sort_column = IDC_CHILDREN_STATISTICS == control_id ? _last_children_sort_column : _last_sort_column;
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmh;
		HWND header = ListView_GetHeader(view);
		HDITEM header_item = { 0 };

		header_item.mask = HDI_FORMAT;
		header_item.fmt = HDF_STRING;

		statistics::sort_predicate predicate = _sorters[pnmlv->iSubItem].first;
		sort_ascending = pnmlv->iSubItem != last_sort_column ? _sorters[pnmlv->iSubItem].second : !sort_ascending;
		if (pnmlv->iSubItem != last_sort_column)
			Header_SetItem(header, last_sort_column, &header_item);
		header_item.fmt = header_item.fmt | (sort_ascending ? HDF_SORTUP : HDF_SORTDOWN);
		last_sort_column = pnmlv->iSubItem;
		Header_SetItem(header, last_sort_column, &header_item);
		if (IDC_CHILDREN_STATISTICS == control_id)
			_statistics.sort_children(predicate, sort_ascending);
		else
			_statistics.sort(predicate, sort_ascending);
		view.Invalidate(FALSE);
		handled = TRUE;
		return 0;
	}

	LRESULT ProfilerMainDialog::OnFocusedFunctionChange(int /*control_id*/, LPNMHDR pnmh, BOOL &handled)
	{
		const NMLISTVIEW *item = (const NMLISTVIEW *)pnmh;

		handled = TRUE;
		if (item->iItem == -1 || ((item->uOldState & LVIS_SELECTED) && !(item->uNewState & LVIS_SELECTED)))
			_statistics.remove_focus(), _last_selected = 0;
		else if (!(item->uOldState & LVIS_SELECTED) && (item->uNewState & LVIS_SELECTED))
			_statistics.set_focus(item->iItem), _last_selected = _statistics.at(item->iItem).first;
		else
			return 0;
		ListView_SetItemCountEx(_children_statistics_view, _statistics.size_children(), 0);
		return 0;
	}

	LRESULT ProfilerMainDialog::OnDrillDown(int /*control_id*/, LPNMHDR pnmh, BOOL &handled)
	{
		const NMITEMACTIVATE *item = (const NMITEMACTIVATE *)pnmh;
		const statistics::statistics_entry &child = _statistics.at_children(item->iItem);

		SelectByAddress(child.first);
		handled = TRUE;
		return 0;
	}

	LRESULT ProfilerMainDialog::OnClearStatistics(WORD /*code*/, WORD /*control_id*/, HWND /*control*/, BOOL &handled)
	{
		_last_selected = 0;
		_statistics.clear();
		ListView_SetItemCountEx(_statistics_view, 0, 0);
		ListView_SetItemCountEx(_children_statistics_view, 0, 0);
		handled = TRUE;
		return 0;
	}

	LRESULT ProfilerMainDialog::OnCopyAll(WORD /*code*/, WORD /*control_id*/, HWND /*control*/, BOOL &handled)
	{
		basic_stringstream<TCHAR> s;

		s << _T("Function\tTimes Called\tExclusive Time\tInclusive Time\tAverage Call Time (Exclusive)\tAverage Call Time (Inclusive)\tMax Recursion") << endl;
		for (size_t i = 0, count = _statistics.size(); i != count; ++i)
		{
			const statistics::statistics_entry &f = _statistics.at(i);

			s << _resolver.symbol_name_by_va(f.first) << _T("\t") << f.second.times_called << _T("\t") << 1.0 * f.second.exclusive_time / g_ticks_resolution << _T("\t") << 1.0 * f.second.inclusive_time / g_ticks_resolution << _T("\t")
				<< 1.0 * f.second.exclusive_time / g_ticks_resolution / f.second.times_called << _T("\t") << 1.0 * f.second.inclusive_time / g_ticks_resolution / f.second.times_called << _T("\t") << f.second.max_reentrance
				<< endl;
		}

		tstring result(s.str());

		if (OpenClipboard())
		{
			if (HGLOBAL gtext = ::GlobalAlloc(GMEM_MOVEABLE, (result.size() + 1) * sizeof(TCHAR)))
			{
				TCHAR *gtext_memory = reinterpret_cast<TCHAR *>(::GlobalLock(gtext));

				copy(result.c_str(), result.c_str() + result.size() + 1, gtext_memory);
				::GlobalUnlock(gtext_memory);
				::EmptyClipboard();
				::SetClipboardData(CF_TEXT, gtext);
			}
			CloseClipboard();
		}
 
		handled = TRUE;
		return 0;
	}

	void ProfilerMainDialog::SelectByAddress(const void *address)
	{
		size_t index = _statistics.find_index(address);

		ListView_SetItemState(_statistics_view, index, LVIS_SELECTED, LVIS_SELECTED);
	}

	void ProfilerMainDialog::RelocateControls(const CSize &size)
	{
		const int spacing = 7;
		CRect rcButton, rc(CPoint(0, 0), size), rcChildren;

		_clear_button.GetWindowRect(&rcButton);
		_children_statistics_view.GetWindowRect(&rcChildren);
		rc.DeflateRect(spacing, spacing, spacing, spacing + rcButton.Height());
		rcButton.MoveToXY(rc.left, rc.bottom);
		_clear_button.MoveWindow(rcButton);
		rcButton.MoveToX(rcButton.Width() + spacing);
		_copy_all_button.MoveWindow(rcButton);
		rc.DeflateRect(0, 0, 0, spacing);
		rcChildren.left = rc.left, rcChildren.right = rc.right;
		rcChildren.MoveToY(rc.bottom - rcChildren.Height());
		rc.DeflateRect(0, 0, 0, spacing + rcChildren.Height());
		_children_statistics_view.MoveWindow(rcChildren);
		_statistics_view.MoveWindow(rc);
	}
}
