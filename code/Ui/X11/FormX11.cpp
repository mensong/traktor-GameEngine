#include <cstring>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/X11/FormX11.h"

namespace traktor
{
	namespace ui
	{

FormX11::FormX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IForm >(context, owner)
{
}

bool FormX11::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	T_FATAL_ASSERT(parent == nullptr);

	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	Window window = XCreateWindow(
		m_context->getDisplay(),
		DefaultRootWindow(m_context->getDisplay()),
		0,
		0,
		width,
		height,
		0,
		0,
		InputOutput,
		CopyFromParent,
		0,
		nullptr
	);

	// Register "delete window" window manager message.
	m_atomWmDeleteWindow = XInternAtom(m_context->getDisplay(), "WM_DELETE_WINDOW", False);

	m_context->bind(&m_data, ClientMessage, [&](XEvent& xe){
		if ((Atom)xe.xclient.data.l[0] == m_atomWmDeleteWindow)
		{
			CloseEvent closeEvent(m_owner);
			m_owner->raiseEvent(&closeEvent);
			if (!(closeEvent.consumed() && closeEvent.cancelled()))
				destroy();
		}		
	});

	XSetWMProtocols(m_context->getDisplay(), window, &m_atomWmDeleteWindow, 1);

	if (!WidgetX11Impl< IForm >::create(nullptr, style, window, Rect(0, 0, width, height), false, true))
		return false;

	setText(text);
	return true;
}

void FormX11::setText(const std::wstring& text)
{
	WidgetX11Impl< IForm >::setText(text);

	std::string cs = wstombs(text);
	const char* csp = cs.c_str();

	XTextProperty tp;
	XStringListToTextProperty((char**)&csp, 1, &tp);

	XSetWMName(m_context->getDisplay(), m_data.window, &tp);
}

void FormX11::setIcon(ISystemBitmap* icon)
{
	Ref< drawing::Image > ii = icon->getImage();
	ii->convert(drawing::PixelFormat::getA8R8G8B8());
	
	const int32_t szs[] = { 16, 32, 64, 128, 256, 0 };
	AlignedVector< unsigned long > data;

	for (const int32_t* sz = szs; *sz != 0; ++sz)
	{
		int32_t w = *sz;
		int32_t h = *sz;

		drawing::ScaleFilter sf(
			w,
			h,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);

		Ref< drawing::Image > img = ii->clone();
		img->apply(&sf);

		data.push_back(w);
		data.push_back(h);

		uint32_t o = data.size();
		data.resize(o + w * h);

		const uint32_t* src = static_cast< const uint32_t* >(img->getData());
		for (uint32_t i = 0; i < w * h; ++i)
			data[o + i] = (unsigned long)src[i];
	}

	XChangeProperty(
		m_context->getDisplay(),
		m_data.window,
		XInternAtom(m_context->getDisplay(), "_NET_WM_ICON", False),
		XA_CARDINAL, 32,
		PropModeReplace,
		(unsigned char*)data.ptr(),
		data.size()
	);

	XFlush(m_context->getDisplay());
}

void FormX11::maximize()
{
}

void FormX11::minimize()
{
}

void FormX11::restore()
{
}

bool FormX11::isMaximized() const
{
	return false;
}

bool FormX11::isMinimized() const
{
	return false;
}

void FormX11::hideProgress()
{
}

void FormX11::showProgress(int32_t current, int32_t total)
{
}

	}
}

