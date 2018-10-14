#include <fontconfig/fontconfig.h>
#include <X11/Xresource.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/X11/BitmapX11.h"
#include "Ui/X11/ContainerX11.h"
#include "Ui/X11/ClipboardX11.h"
#include "Ui/X11/DialogX11.h"
#include "Ui/X11/EventLoopX11.h"
#include "Ui/X11/FormX11.h"
#include "Ui/X11/ToolFormX11.h"
#include "Ui/X11/UserWidgetX11.h"
#include "Ui/X11/WidgetFactoryX11.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

int xerrorHandler(Display*, XErrorEvent* ee)
{
	log::error << L"X11 error occured, major code " << (int32_t)ee->error_code << Endl;
	return 0;
}

		}

WidgetFactoryX11::WidgetFactoryX11()
:	m_dpi(96)
{
	XIM xim;

	// Open connection to X display.
	Display* display = XOpenDisplay(nullptr);
	T_FATAL_ASSERT(display != nullptr);

    int screen = DefaultScreen(display);

	// Use our own X error handler.
	XSetErrorHandler(xerrorHandler);

	// Get DPI from X, as dpi handling in X is severly broken we deduce either small or large only.
	float wdots = (float)XDisplayWidth(display, screen);
	float winch = (float)XDisplayWidthMM(display, screen) * 0.0393700787f;
	if (wdots / winch > 96.0f)
		m_dpi = 192;
	else
		m_dpi = 96;

	// Open input method.
	XSetLocaleModifiers("");
	if ((xim = XOpenIM(display, nullptr, nullptr, nullptr)) == 0)
	{
		XSetLocaleModifiers("@im=");
		if ((xim = XOpenIM(display, nullptr, nullptr, nullptr)) == 0)
		{
			log::error << L"Unable to open X11 input method." << Endl;
		}
	}

	// Create our context.
	m_context = new Context(display, screen, xim);
}

WidgetFactoryX11::~WidgetFactoryX11()
{
	// if (m_xim != 0)
	// {
	// 	XCloseIM(m_xim);
	// 	m_xim = 0;
	// }
	
	// if (m_context->getDisplay() != nullptr)
	// {
	// 	XCloseDisplay(m_context->getDisplay());
	// 	m_context->getDisplay() = nullptr;
	// }

	m_context = nullptr;
}

IEventLoop* WidgetFactoryX11::createEventLoop(EventSubject* owner)
{
	return new EventLoopX11(m_context);
}

IContainer* WidgetFactoryX11::createContainer(EventSubject* owner)
{
	return new ContainerX11(m_context, owner);
}

IDialog* WidgetFactoryX11::createDialog(EventSubject* owner)
{
	return new DialogX11(m_context, owner);
}

IForm* WidgetFactoryX11::createForm(EventSubject* owner)
{
	return new FormX11(m_context, owner);
}

INotificationIcon* WidgetFactoryX11::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPathDialog* WidgetFactoryX11::createPathDialog(EventSubject* owner)
{
	return 0;
}

IToolForm* WidgetFactoryX11::createToolForm(EventSubject* owner)
{
	return new ToolFormX11(m_context, owner);
}

IUserWidget* WidgetFactoryX11::createUserWidget(EventSubject* owner)
{
	return new UserWidgetX11(m_context, owner);
}

IWebBrowser* WidgetFactoryX11::createWebBrowser(EventSubject* owner)
{
	return 0;
}

ISystemBitmap* WidgetFactoryX11::createBitmap()
{
	return new BitmapX11();
}

IClipboard* WidgetFactoryX11::createClipboard()
{
	return new ClipboardX11();
}

int32_t WidgetFactoryX11::getSystemDPI() const
{
    return m_dpi;
}

void WidgetFactoryX11::getSystemFonts(std::list< std::wstring >& outFonts)
{
	FcConfig* config = FcInitLoadConfigAndFonts();
	if (config == nullptr)
		return;

	FcPattern* pat = FcPatternCreate();
	if (pat == nullptr)
		return;

	FcObjectSet* os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, (char*)nullptr);
	if (os == nullptr)
		return;

	FcFontSet* fs = FcFontList(config, pat, os);
	if (fs == nullptr)
		return;

	for (int i = 0; fs && i < fs->nfont; ++i)
	{
		FcPattern* font = fs->fonts[i];
		if (font == nullptr)
			continue;

		FcChar8 *family;
		if (FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch)
		{
			outFonts.push_back(mbstows((const char*)family));
		}
	}

	FcFontSetDestroy(fs);
}

void WidgetFactoryX11::getDesktopRects(std::list< Rect >& outRects) const
{
	int32_t width = DisplayWidth(m_context->getDisplay(), m_context->getScreen());
	int32_t height = DisplayHeight(m_context->getDisplay(), m_context->getScreen());
	outRects.push_back(Rect(
		0, 0,
		width, height
	));
}

	}
}

