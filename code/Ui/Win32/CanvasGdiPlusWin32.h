#ifndef traktor_ui_CanvasGdiPlusWin32_H
#define traktor_ui_CanvasGdiPlusWin32_H

#if defined(T_USE_GDI_PLUS)

#include <windows.h>
#include <gdiplus.h>
#include "Ui/Win32/CanvasWin32.h"

namespace traktor
{
	namespace ui
	{

class CanvasGdiPlusWin32 : public CanvasWin32
{
public:
	CanvasGdiPlusWin32();

	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC);

	virtual void endPaint(Window& hWnd);
	
	virtual void setForeground(const Color& foreground);

	virtual void setBackground(const Color& background);

	virtual void setFont(const Font& font);

	virtual void setLineStyle(LineStyle lineStyle);

	virtual void setPenThickness(int thickness);

	virtual void setClipRect(const Rect& rc);

	virtual void resetClipRect();
	
	virtual void drawPixel(int x, int y, const Color& c);

	virtual void drawLine(int x1, int y1, int x2, int y2);

	virtual void drawLines(const Point* pnts, int npnts);

	virtual void fillCircle(int x, int y, float radius);

	virtual void drawCircle(int x, int y, float radius);

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end);

	virtual void drawSpline(const Point* pnts, int npnts);

	virtual void fillRect(const Rect& rc);

	virtual void fillGradientRect(const Rect& rc, bool vertical = true);

	virtual void drawRect(const Rect& rc);

	virtual void drawRoundRect(const Rect& rc, int radius);

	virtual void drawPolygon(const Point* pnts, int count);

	virtual void fillPolygon(const Point* pnts, int count);
	
	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, BlendMode blendMode);

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, BlendMode blendMode);

	virtual void drawText(const Point& at, const std::wstring& text);

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop);
	
	virtual Size getTextExtent(const std::wstring& text) const;

	static bool startup();

	static void shutdown();
	
private:
	PAINTSTRUCT m_ps;
	HDC m_hDC;
	bool m_ownDC;
	HBITMAP m_hOffScreenBitmap;

	Gdiplus::Graphics* m_graphics;
	Gdiplus::Color m_foreGround;
	Gdiplus::Color m_backGround;
	Gdiplus::Pen m_pen;
	Gdiplus::SolidBrush m_brush;
	Gdiplus::Font* m_font;
};
	
	}
}

#endif	// T_USE_GDI_PLUS

#endif	// traktor_ui_CanvasGdiPlusWin32_H
