#include "Ui/Win32/Window.h"
#include "Core/Log/Log.h"

extern HINSTANCE g_hInstance;

#if defined(WINCE)
#	define GWLP_USERDATA GWL_USERDATA
#	define GWLP_WNDPROC GWL_WNDPROC
#	define SET_WINDOW_LONG_PTR(a, b, c) SetWindowLong(a, b, c)
#	define GET_WINDOW_LONG_PTR(a, b) GetWindowLong(a, b)
#else
#	define SET_WINDOW_LONG_PTR(a, b, c) SetWindowLongPtr(a, b, c)
#	define GET_WINDOW_LONG_PTR(a, b) GetWindowLongPtr(a, b)
#endif

namespace traktor
{
	namespace ui
	{

Window::Window()
:	m_hWnd(0)
,	m_hFont(0)
,	m_originalWndProc(0)
{
}

Window::~Window()
{
	if (m_originalWndProc)
		SET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_originalWndProc);

	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)0);

#if !defined(WINCE)
	DeleteObject(m_hFont);
#endif

	DestroyWindow(m_hWnd);
}

bool Window::create(
	HWND hParentWnd,
	LPCTSTR className,
	LPCTSTR text,
	UINT style,
	UINT styleEx,
	int left,
	int top,
	int width,
	int height,
	int id,
	bool subClass
)
{
	T_ASSERT_M (!m_hWnd, L"Window already created");

	m_hWnd = CreateWindowEx(
		styleEx,
		className,
		text,
		style,
		left,
		top,
		width,
		height,
		hParentWnd,
		(HMENU)id,
		g_hInstance,
		NULL
	);
	if (!m_hWnd)
	{
		log::error << L"Unable to create window" << Endl;
		return false;
	}

#if !defined(WINCE)
	if (_tcscmp(className, _T("TraktorDialogWin32Class")) != 0)
		SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
	else
		SET_WINDOW_LONG_PTR(m_hWnd, DWLP_USER, (LONG_PTR)this);
#else
	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
#endif

	if (subClass)
	{
		m_originalWndProc = (WNDPROC)GET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC);
		SET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC, (LONG_PTR)wndProcSubClass);
	}

#if !defined(WINCE)

	ICONMETRICS im;
	memset(&im, 0, sizeof(im));
	im.cbSize = sizeof(im);
	SystemParametersInfo(SPI_GETICONMETRICS, 0, &im, sizeof(im));
	m_hFont = CreateFontIndirect(&im.lfFont);

#else
	m_hFont = (HFONT)GetStockObject(SYSTEM_FONT);
#endif
	SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hFont, FALSE);

	return true;
}

bool Window::subClass(HWND hWnd)
{
	T_ASSERT (!m_hWnd);

	m_hWnd = hWnd;
	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	m_originalWndProc = (WNDPROC)GET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC);
	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC, (LONG_PTR)wndProcSubClass);

	return true;
}

void Window::setFont(HFONT hFont)
{
	HFONT hOldFont = m_hFont;

	m_hFont = hFont;
	sendMessage(WM_SETFONT, (WPARAM)m_hFont, TRUE);

	if (hOldFont)
		DeleteObject(hOldFont);
}

HFONT Window::getFont() const
{
	return m_hFont;
}

LRESULT Window::sendMessage(UINT message, WPARAM wParam, LPARAM lParam) const
{
	return SendMessage(m_hWnd, message, wParam, lParam);
}

Window::operator HWND () const
{
	return m_hWnd;
}

void Window::registerMessageHandler(UINT message, IMessageHandler* messageHandler)
{
	m_messageHandlers[message] = messageHandler;
}

void Window::registerDefaultClass()
{
	WNDCLASS wc;

	std::memset(&wc, 0, sizeof(wc));
	wc.style         = CS_DBLCLKS;
	wc.lpfnWndProc   = wndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	wc.hInstance     = g_hInstance;
#if !defined(WINCE)
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
#else
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#endif
	wc.hIcon         = LoadIcon(g_hInstance, _T("DEFAULTICON"));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("TraktorWin32Class");

	if (!RegisterClass(&wc))
		log::error << L"Unable to register class \"TraktorWin32Class\"" << Endl;
}

void Window::unregisterDefaultClass()
{
	UnregisterClass(_T("TraktorWin32Class"), g_hInstance);
}

void Window::registerDialogClass()
{
	WNDCLASS wc;

	std::memset(&wc, 0, sizeof(wc));
	wc.style         = CS_DBLCLKS;
	wc.lpfnWndProc   = dlgProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = DLGWINDOWEXTRA + sizeof(void*);
	wc.hInstance     = g_hInstance;
#if !defined(WINCE)
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
#else
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#endif
	wc.hIcon         = LoadIcon(g_hInstance, _T("DEFAULTICON"));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("TraktorDialogWin32Class");

	if (!RegisterClass(&wc))
		log::error << L"Unable to register class \"TraktorDialogWin32Class\"" << Endl;
}

void Window::unregisterDialogClass()
{
	UnregisterClass(_T("TraktorDialogWin32Class"), g_hInstance);
}

LRESULT Window::invokeMessageHandlers(HWND hWnd, DWORD dwIndex, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	LRESULT result = FALSE;

	Window* window = reinterpret_cast< Window* >(GET_WINDOW_LONG_PTR(hWnd, dwIndex));
	if (window)
	{
		Ref< IMessageHandler > messageHandler = window->m_messageHandlers[message];
		if (messageHandler)
		{
			pass = false;
			result = messageHandler->handle(hWnd, message, wParam, lParam, pass);
		}
		else
			pass = true;
	}
	else
		pass = true;

	return result;
}

LRESULT CALLBACK Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;
	bool pass;

	// Lookup handler of issued message.
	result = invokeMessageHandlers(hWnd, GWLP_USERDATA, message, wParam, lParam, pass);
	if (!pass)
		return result;

	// Reflect messages sent to parents back to issuing child.
	if (message == WM_COMMAND)
	{
		HWND hWndControl = (HWND)lParam;
		if (hWndControl)
		{
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, WM_REFLECTED_COMMAND, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if (message == WM_NOTIFY)
	{
		LPNMHDR nmhdr = reinterpret_cast< LPNMHDR >(lParam);
		if (nmhdr && nmhdr->hwndFrom)
		{
			result = invokeMessageHandlers(nmhdr->hwndFrom, GWLP_USERDATA, WM_REFLECTED_NOTIFY, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if (message == WM_HSCROLL || message == WM_VSCROLL)
	{
		HWND hWndControl = (HWND)lParam;
		if (hWndControl)
		{
			UINT reflectMsg = (message == WM_HSCROLL) ? WM_REFLECTED_HSCROLL : WM_REFLECTED_VSCROLL;
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, reflectMsg, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}

	// Call default window procedure.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK Window::dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;
	bool pass;

	// Lookup handler of issued message.
#if !defined(WINCE)
	result = invokeMessageHandlers(hWnd, DWLP_USER, message, wParam, lParam, pass);
#else
	result = invokeMessageHandlers(hWnd, GWLP_USERDATA, message, wParam, lParam, pass);
#endif
	if (!pass)
		return result;

	// Reflect messages sent to parents back to issuing child.
	if (message == WM_COMMAND)
	{
		HWND hWndControl = (HWND)lParam;
		if (hWndControl)
		{
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, WM_REFLECTED_COMMAND, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if (message == WM_NOTIFY)
	{
		LPNMHDR nmhdr = reinterpret_cast< LPNMHDR >(lParam);
		if (nmhdr && nmhdr->hwndFrom)
		{
			result = invokeMessageHandlers(nmhdr->hwndFrom, GWLP_USERDATA, WM_REFLECTED_NOTIFY, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if (message == WM_HSCROLL || message == WM_VSCROLL)
	{
		HWND hWndControl = (HWND)lParam;
		if (hWndControl)
		{
			UINT reflectMsg = (message == WM_HSCROLL) ? WM_REFLECTED_HSCROLL : WM_REFLECTED_VSCROLL;
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, reflectMsg, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}

	// Call default dialog procedure.
	return DefDlgProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK Window::wndProcSubClass(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Lookup handler of issued message.
	Window* window = reinterpret_cast< Window* >(GET_WINDOW_LONG_PTR(hWnd, GWLP_USERDATA));
	if (window)
	{
		WNDPROC originalWndProc = window->m_originalWndProc;

		// Call custom message handler.
		IMessageHandler* messageHandler = window->m_messageHandlers[message];
		if (messageHandler)
		{
			bool pass = false;
			LRESULT result = messageHandler->handle(hWnd, message, wParam, lParam, pass);
			if (!pass)
				return result;
		}

		// Call original window procedure.
		if (originalWndProc && IsWindow(hWnd))
			return CallWindowProc(
				originalWndProc,
				hWnd,
				message,
				wParam,
				lParam
			);
	}
	
	// Unable to get original window procedure, call default window procedure.
	return DefWindowProc(
		hWnd,
		message,
		wParam,
		lParam
	);
}

	}
}
