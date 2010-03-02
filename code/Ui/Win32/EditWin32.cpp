#include "Ui/Win32/EditWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_heightMargin = 13;

		}

EditWin32::EditWin32(EventSubject* owner)
:	WidgetWin32Impl< IEdit >(owner)
{
}

bool EditWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("EDIT"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_LEFT | ES_WANTRETURN | nativeStyle,
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

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< EditWin32 >(this, &EditWin32::eventCommand));

	return true;
}

void EditWin32::setSelection(int from, int to)
{
	m_hWnd.sendMessage(EM_SETSEL, (WPARAM)from, (LPARAM)to);
}

void EditWin32::getSelection(int& outFrom, int& outTo) const
{
	m_hWnd.sendMessage(EM_GETSEL, (WPARAM)(LPDWORD)&outFrom, (LPARAM)(LPDWORD)&outTo);
}

void EditWin32::selectAll()
{
	m_hWnd.sendMessage(EM_SETSEL, 0, -1);
}

Size EditWin32::getPreferedSize() const
{
	Font currentFont = getFont();
	int32_t currentFontHeight = abs(currentFont.getSize() * 72) / 96;
	return Size(128, currentFontHeight + c_heightMargin);
}

LRESULT EditWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	if (HIWORD(wParam) != EN_CHANGE)
		return 0;

	CommandEvent cmdEvent(m_owner, 0);
	m_owner->raiseEvent(EiContentChange, &cmdEvent);

	return 0;
}

	}
}
