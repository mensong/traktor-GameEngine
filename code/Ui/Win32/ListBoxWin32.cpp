#include "Ui/Win32/ListBoxWin32.h"
#include "Ui/ListBox.h"

namespace traktor
{
	namespace ui
	{

ListBoxWin32::ListBoxWin32(EventSubject* owner)
:	WidgetWin32Impl< IListBox >(owner)
,	m_single(true)
{
}

bool ListBoxWin32::create(IWidget* parent, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & ListBox::WsMultiple)
	{
		nativeStyle |= LBS_MULTIPLESEL;
		m_single = false;
	}
	if (style & ListBox::WsExtended)
	{
		nativeStyle |= LBS_EXTENDEDSEL;
		m_single = false;
	}
	if (style & ListBox::WsSort)
		nativeStyle |= LBS_SORT;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("LISTBOX"),
		_T(""),
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_TABSTOP | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< ListBoxWin32 >(this, &ListBoxWin32::eventCommand));

	return true;
}

int ListBoxWin32::add(const std::wstring& item)
{
	return (int)m_hWnd.sendMessage(LB_ADDSTRING, 0, (LPARAM)wstots(item).c_str());
}

bool ListBoxWin32::remove(int index)
{
	return bool(m_hWnd.sendMessage(LB_DELETESTRING, (WPARAM)index, 0) != LB_ERR);
}

void ListBoxWin32::removeAll()
{
	m_hWnd.sendMessage(LB_RESETCONTENT, 0, 0);
}

int ListBoxWin32::count() const
{
	return (int)m_hWnd.sendMessage(LB_GETCOUNT, 0, 0);
}

void ListBoxWin32::set(int index, const std::wstring& item)
{
	remove(index);
	m_hWnd.sendMessage(LB_INSERTSTRING, (WPARAM)index, (LPARAM)wstots(item).c_str());
}

std::wstring ListBoxWin32::get(int index) const
{
	int len = (int)m_hWnd.sendMessage(LB_GETTEXTLEN, (WPARAM)index, 0);
	if (len <= 0 || len == LB_ERR)
		return L"";

	std::vector< TCHAR > tmp(len + 1);
	m_hWnd.sendMessage(LB_GETTEXT, (WPARAM)index, (LPARAM)&tmp[0]);

	return tstows(&tmp[0]);
}

void ListBoxWin32::select(int index)
{
	if (m_single)
		m_hWnd.sendMessage(LB_SETCURSEL, (WPARAM)index, 0);
	else
		m_hWnd.sendMessage(LB_SETSEL, (WPARAM)TRUE, (LPARAM)index);
}

bool ListBoxWin32::selected(int index) const
{
	return bool(m_hWnd.sendMessage(LB_GETSEL, (WPARAM)index, 0) > 0);
}

Rect ListBoxWin32::getItemRect(int index) const
{
	RECT rc;
	m_hWnd.sendMessage(
		LB_GETITEMRECT,
		(WPARAM)index,
		(LPARAM)&rc
	);
	return Rect(rc.left, rc.top, rc.right, rc.bottom);
}

LRESULT ListBoxWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	skip = true;
	if (HIWORD(wParam) == LBN_SELCHANGE)
	{
		int selected = int(m_hWnd.sendMessage(LB_GETCURSEL, 0, 0));
		if (selected == LB_ERR)
			selected = -1;

		CommandEvent cmdEvent(m_owner, 0, Command(selected));
		m_owner->raiseEvent(EiSelectionChange, &cmdEvent);
		if (cmdEvent.consumed())
			skip = false;
	}
	return TRUE;
}

	}
}
