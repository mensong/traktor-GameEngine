#ifndef traktor_ui_WidgetWxImpl_H
#define traktor_ui_WidgetWxImpl_H

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string>
#include <map>
#include <wx/wx.h>
#include <wx/event.h>
#if defined(__WXGTK__)
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <wx/gtk/win_gtk.h>
#endif
#include "Ui/Widget.h"
#include "Ui/ShowEvent.h"
#include "Ui/KeyEvent.h"
#include "Ui/MoveEvent.h"
#include "Ui/SizeEvent.h"
#include "Ui/MouseEvent.h"
#include "Ui/FocusEvent.h"
#include "Ui/CommandEvent.h"
#include "Ui/PaintEvent.h"
#include "Ui/Canvas.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Wx/EvtHandler.h"
#include "Ui/Wx/CanvasWx.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

/**
 * Common implementation of WxWindow widgets.
 */
template < typename ControlType, typename WxWindowType >
class WidgetWxImpl : public ControlType
{
public:
	WidgetWxImpl(EventSubject* owner)
	:	m_owner(owner)
	,	m_window(0)
	,	m_doubleBuffer(false)
	,	m_canvasImpl(0)
	{
	}

	virtual void destroy()
	{
		for (std::map< int, wxTimer* >::iterator i = m_timers.begin(); i != m_timers.end(); ++i)
			delete i->second;

		m_timers.clear();

		if (m_window)
			while (m_window->Disconnect());

		if (m_canvasImpl)
		{
			delete m_canvasImpl;
			m_canvasImpl = 0;
		}

		if (m_window)
		{
			m_window->Destroy();
			m_window = 0;
		}

		delete this;
	}

	virtual void setStyle(int style)
	{
		int wxStyle = wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE;

		if (style & WsBorder)
			wxStyle |= wxSIMPLE_BORDER;
		if (style & WsClientBorder)
			wxStyle |= wxSUNKEN_BORDER;
		if (style & WsResizable)
			wxStyle |= wxRESIZE_BORDER;
		if (style & WsSystemBox)
			wxStyle |= wxSYSTEM_MENU;
		if (style & WsMinimizeBox)
			wxStyle |= wxMINIMIZE_BOX;
		if (style & WsMaximizeBox)
			wxStyle |= wxMAXIMIZE_BOX;
		if (style & WsCloseBox)
			wxStyle |= wxCLOSE_BOX;
		if (style & WsCaption)
			wxStyle |= wxCAPTION;
		if (style & WsTop)
			wxStyle |= wxSTAY_ON_TOP;

		if (style & WsDoubleBuffer)
			m_doubleBuffer = true;
		else
			m_doubleBuffer = false;

		m_window->SetWindowStyle(style);
	}

	virtual void setParent(IWidget* parent)
	{
		m_window->Reparent(static_cast< wxWindow* >(parent->getInternalHandle()));
	}

	virtual void setText(const std::wstring& text)
	{
		tstring tmp = wstots(text);
		m_window->SetLabel(tmp.c_str());
	}

	virtual std::wstring getText() const
	{
		tstring tmp = (const wxChar*)m_window->GetLabel();
		return tstows(tmp);
	}

	virtual void setToolTipText(const std::wstring& text)
	{
		tstring tmp = wstots(text);
		m_window->SetToolTip(tmp.c_str());
	}

	virtual void setForeground()
	{
		// @fixme
	}

	virtual bool isForeground() const
	{
		// @fixme
		return true;
	}

	virtual void setVisible(bool visible)
	{
		if (visible ^ m_window->IsShown())
		{
			m_window->Show(visible);
			
			ShowEvent showEvent(m_owner, 0, visible);
			m_owner->raiseEvent(EiShow, &showEvent);
		}
	}

	virtual bool isVisible(bool includingParents) const
	{
		if (includingParents)
			return m_window->IsShownOnScreen();
		else
			return m_window->IsShown();
	}

	virtual void setActive()
	{
		// @fixme
	}

	virtual void setEnable(bool enable)
	{
		m_window->Enable(enable);
	}

	virtual bool isEnable() const
	{
		return m_window->IsEnabled();
	}

	virtual bool hasFocus() const
	{
		return bool(wxWindow::FindFocus() == m_window);
	}

	virtual void setFocus()
	{
		m_window->SetFocus();
	}

	virtual bool hasCapture() const
	{
		return m_window->HasCapture();
	}

	virtual void setCapture()
	{
		m_window->CaptureMouse();
	}

	virtual void releaseCapture()
	{
		if (m_window->HasCapture())
			m_window->ReleaseMouse();
	}

	virtual void startTimer(int interval, int id)
	{
		stopTimer(id);
		m_timers[id] = new wxTimer(m_window, id);
		m_timers[id]->Start(interval);
	}

	virtual void stopTimer(int id)
	{
		if (m_timers[id])
		{
			m_timers[id]->Stop();
			delete m_timers[id];
			m_timers[id] = 0;
		}
	}

	virtual void setOutline(const Point* p, int np)
	{
	}

	virtual void setRect(const Rect& rect)
	{
		m_window->SetSize(rect.left, rect.top, rect.getWidth(), rect.getHeight());
	}

	virtual Rect getRect() const
	{
		wxRect rect = m_window->GetRect();
		return Rect(rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
	}

	virtual Rect getInnerRect() const
	{
		wxRect rect = m_window->GetClientRect();
		return Rect(0, 0, rect.GetWidth(), rect.GetHeight());
	}

	virtual Rect getNormalRect() const
	{
#if defined(_WIN32) && !defined(WINCE)
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof(wp));
		wp.length = sizeof(wp);
		GetWindowPlacement((HWND)m_window->GetHWND(), &wp);
		const RECT rc = wp.rcNormalPosition;
		return Rect(rc.left, rc.top, rc.right, rc.bottom);
#else
		// wxWidgets doesn't expose this functionality, thus we fake it.
		wxRect rect = m_window->GetRect();
		return Rect(rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
#endif
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		Size s;
		tstring tmp = wstots(text);
		m_window->GetTextExtent(tmp.c_str(), &s.cx, &s.cy);
		return s;
	}

	virtual void setFont(const Font& font)
	{
		m_window->SetFont(wxFont(
			font.getSize(),
			wxFONTFAMILY_DEFAULT,
			font.isItalic() ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			font.isBold() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
			font.isUnderline(),
			wstots(font.getFace()).c_str(),
			wxFONTENCODING_DEFAULT
		));
	}

	virtual Font getFont() const
	{
		wxFont font = m_window->GetFont();
		return Font(
			tstows(static_cast< const wxChar* >(font.GetFaceName())),
			font.GetPointSize(),
			bool(font.GetWeight() == wxFONTWEIGHT_BOLD),
			bool(font.GetStyle() == wxFONTSTYLE_ITALIC),
			font.GetUnderlined()
		);
	}

	virtual void setCursor(Cursor cursor)
	{
	}

	virtual Point getMousePosition(bool relative) const
	{
		Point pt;
		wxGetMousePosition(&pt.x, &pt.y);
		return relative ? screenToClient(pt) : pt;
	}

	virtual Point screenToClient(const Point& pt) const
	{
		Point ptc = pt;
		m_window->ScreenToClient(&ptc.x, &ptc.y);
		return ptc;
	}

	virtual Point clientToScreen(const Point& pt) const
	{
		Point pts = pt;
		m_window->ClientToScreen(&pts.x, &pts.y);
		return pts;
	}

	virtual bool hitTest(const Point& pt) const
	{
		return false;
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects)
	{
		m_window->Freeze();
		for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
		{
			wxWindow* widget = static_cast< wxWindow* >(i->widget->getInternalHandle());
			widget->SetSize(
				i->rect.left,
				i->rect.top,
				i->rect.getWidth(),
				i->rect.getHeight()
			);
		}
		m_window->Thaw();
	}

	virtual Size getMinimumSize() const
	{
		wxSize s = m_window->GetMinSize();
		return Size(s.x, s.y);
	}

	virtual Size getPreferedSize() const
	{
		wxSize s = m_window->GetBestSize();
		return Size(s.x, s.y);
	}

	virtual Size getMaximumSize() const
	{
		return Size(65536, 65536);
	}

	virtual void update(const Rect* rc, bool immediate)
	{
		if (!rc)
			m_window->Refresh(false);
		else
		{
			wxRect refreshRc(rc->left, rc->top, rc->getWidth(), rc->getHeight());
			m_window->Refresh(false, &refreshRc);
		}
		if (immediate)
			m_window->Update();
	}

	virtual void* getInternalHandle()
	{
		return m_window;
	}

	virtual void* getSystemHandle()
	{
#if defined(__WXMSW__)
		return m_window->GetHWND();
#elif defined(__WXMAC__)
		return m_window->GetHandle();	// Always returns ControlRef handle.
#else
		// Get internal GTK widget from wxGTK.
		GtkWidget* internalWidget = m_window->m_wxwindow;
		T_ASSERT (internalWidget);

		// Hack, disable GTK double buffering. This method is currently
		// only called when creating a render widget.
		gtk_widget_set_double_buffered(internalWidget, FALSE);

		GdkWindow* gw = GTK_PIZZA(internalWidget)->bin_window;
		Display* display = GDK_WINDOW_XDISPLAY(gw);
		Window window = GDK_WINDOW_XWINDOW(gw);

		// @fixme, Ouch, return pointer to static buffer.
		static struct { Display* display; Window window; } handle = { display, window };
		return (void*)&handle;
#endif
	}

protected:
	EventSubject* m_owner;
	WxWindowType* m_window;
	std::map< int, wxTimer* > m_timers;
	bool m_doubleBuffer;
	CanvasWx* m_canvasImpl;

	bool create(int style)
	{
		if (!m_window)
			return false;

		if (style & WsDoubleBuffer)
			m_doubleBuffer = true;

		m_canvasImpl = new CanvasWx();

		T_CONNECT(m_window, wxEVT_CHAR,             wxKeyEvent,   WidgetWxImpl, &WidgetWxImpl::onChar);
		T_CONNECT(m_window, wxEVT_KEY_DOWN,         wxKeyEvent,   WidgetWxImpl, &WidgetWxImpl::onKeyDown);
		T_CONNECT(m_window, wxEVT_KEY_UP,           wxKeyEvent,   WidgetWxImpl, &WidgetWxImpl::onKeyUp);
		T_CONNECT(m_window, wxEVT_MOVE,             wxMoveEvent,  WidgetWxImpl, &WidgetWxImpl::onMove);
		T_CONNECT(m_window, wxEVT_SIZE,             wxSizeEvent,  WidgetWxImpl, &WidgetWxImpl::onSize);
		T_CONNECT(m_window, wxEVT_LEFT_DOWN,        wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseDown);
		T_CONNECT(m_window, wxEVT_LEFT_UP,          wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseUp);
		T_CONNECT(m_window, wxEVT_LEFT_DCLICK,      wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseDoubleClick);
		T_CONNECT(m_window, wxEVT_RIGHT_DOWN,       wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseDown);
		T_CONNECT(m_window, wxEVT_RIGHT_UP,         wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseUp);
		T_CONNECT(m_window, wxEVT_RIGHT_DCLICK,     wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseDoubleClick);
		T_CONNECT(m_window, wxEVT_MOTION,           wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseMotion);
		T_CONNECT(m_window, wxEVT_MOUSEWHEEL,       wxMouseEvent, WidgetWxImpl, &WidgetWxImpl::onMouseWheel);
		T_CONNECT(m_window, wxEVT_SET_FOCUS,        wxFocusEvent, WidgetWxImpl, &WidgetWxImpl::onSetFocus);
		T_CONNECT(m_window, wxEVT_KILL_FOCUS,       wxFocusEvent, WidgetWxImpl, &WidgetWxImpl::onKillFocus);
		T_CONNECT(m_window, wxEVT_PAINT,            wxPaintEvent, WidgetWxImpl, &WidgetWxImpl::onPaint);
		T_CONNECT(m_window, wxEVT_ERASE_BACKGROUND, wxEraseEvent, WidgetWxImpl, &WidgetWxImpl::onEraseBackground);
		T_CONNECT(m_window, wxEVT_TIMER,            wxTimerEvent, WidgetWxImpl, &WidgetWxImpl::onTimer);

		return true;
	}

	void onChar(wxKeyEvent& event)
	{
		KeyEvent k(m_owner, 0, event.GetKeyCode());
		m_owner->raiseEvent(EiKey, &k);

		event.Skip(true);
	}

	void onKeyDown(wxKeyEvent& event)
	{
		KeyEvent k(m_owner, 0, event.GetKeyCode());
		m_owner->raiseEvent(EiKeyDown, &k);

		event.Skip(true);
	}

	void onKeyUp(wxKeyEvent& event)
	{
		KeyEvent k(m_owner, 0, event.GetKeyCode());
		m_owner->raiseEvent(EiKeyUp, &k);

		event.Skip(true);
	}

	void onMove(wxMoveEvent& event)
	{
		wxPoint pt = event.GetPosition();

		MoveEvent m(m_owner, 0, Point(pt.x, pt.y));
		m_owner->raiseEvent(EiMove, &m);

		if (!m.consumed())
			event.Skip(true);
	}

	void onSize(wxSizeEvent& event)
	{
		wxSize sz = event.GetSize();

		SizeEvent s(m_owner, 0, Size(sz.x, sz.y));
		m_owner->raiseEvent(EiSize, &s);

		if (!s.consumed())
			event.Skip(true);
	}

	void onMouseDown(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();

		const MouseEvent::Button buttons[] =
		{
			MouseEvent::BtNone,
			MouseEvent::BtLeft,
			MouseEvent::BtMiddle,
			MouseEvent::BtRight
		};

		MouseEvent m(m_owner, 0, buttons[event.GetButton()], Point(p.x, p.y));
		m_owner->raiseEvent(EiButtonDown, &m);

		if (!m.consumed())
			event.Skip(true);
	}

	void onMouseUp(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();

		const MouseEvent::Button buttons[] =
		{
			MouseEvent::BtNone,
			MouseEvent::BtLeft,
			MouseEvent::BtMiddle,
			MouseEvent::BtRight
		};

		MouseEvent m(m_owner, 0, buttons[event.GetButton()], Point(p.x, p.y));
		m_owner->raiseEvent(EiButtonUp, &m);

		if (!m.consumed())
			event.Skip(true);
	}

	void onMouseDoubleClick(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();

		const MouseEvent::Button buttons[] =
		{
			MouseEvent::BtNone,
			MouseEvent::BtLeft,
			MouseEvent::BtMiddle,
			MouseEvent::BtRight
		};

		MouseEvent m(m_owner, 0, buttons[event.GetButton()], Point(p.x, p.y));
		m_owner->raiseEvent(EiDoubleClick, &m);

		if (!m.consumed())
			event.Skip(true);
	}

	void onMouseMotion(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();

		int button = 0;
		if (event.LeftIsDown())
			button |= MouseEvent::BtLeft;
		if (event.MiddleIsDown())
			button |= MouseEvent::BtMiddle;
		if (event.RightIsDown())
			button |= MouseEvent::BtRight;

		MouseEvent m(m_owner, 0, button, Point(p.x, p.y));
		m_owner->raiseEvent(EiMouseMove, &m);

		if (!m.consumed())
			event.Skip(true);
	}

	void onMouseWheel(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();

		int button = 0;
		if (event.LeftIsDown())
			button |= MouseEvent::BtLeft;
		if (event.MiddleIsDown())
			button |= MouseEvent::BtMiddle;
		if (event.RightIsDown())
			button |= MouseEvent::BtRight;

		MouseEvent m(m_owner, 0, button, Point(p.x, p.y), event.GetWheelRotation());
		m_owner->raiseEvent(EiMouseWheel, &m);

		if (!m.consumed())
			event.Skip(true);
	}

	void onSetFocus(wxFocusEvent& event)
	{
		FocusEvent focusEvent(m_owner, 0, true);
		m_owner->raiseEvent(EiFocus, &focusEvent);
		event.Skip(true);
	}

	void onKillFocus(wxFocusEvent& event)
	{
		FocusEvent focusEvent(m_owner, 0, false);
		m_owner->raiseEvent(EiFocus, &focusEvent);
		event.Skip(true);
	}

	void onPaint(wxPaintEvent& event)
	{
		if (!m_owner->hasEventHandler(EiPaint) || !m_canvasImpl)
		{
			// Call default painter.
			event.Skip(true);
			return;
		}

		if (m_canvasImpl->beginPaint(m_window, m_doubleBuffer))
		{
			wxRect rc = m_window->GetUpdateRegion().GetBox();
			Canvas canvas(m_canvasImpl);
			PaintEvent p(
				m_owner,
				0,
				canvas,
				ui::Rect(rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetBottom())
			);
			m_owner->raiseEvent(EiPaint, &p);
			if (!p.consumed())
				event.Skip(true);
			m_canvasImpl->endPaint(m_window);
		}
		else
			event.Skip(true);
	}

	void onEraseBackground(wxEraseEvent& event)
	{
		if (!m_owner->hasEventHandler(EiPaint))
			event.Skip(true);
	}

	void onTimer(wxTimerEvent& event)
	{
		CommandEvent cmdEvent(m_owner, 0);
		m_owner->raiseEvent(EiTimer, &cmdEvent);
		event.Skip(true);
	}
};

	}
}

#endif	// traktor_ui_WidgetWxImpl_H
