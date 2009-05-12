#ifndef traktor_ui_custom_QuadSplitter_H
#define traktor_ui_custom_QuadSplitter_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Quadruple splitter.
 * \ingroup UIC
 */
class T_DLLCLASS QuadSplitter : public Widget
{
	T_RTTI_CLASS(QuadSplitter)

public:
	QuadSplitter();

	/*! \brief Create splitter control.
	 *
	 * \param parent Parent widget.
	 * \param position Initial position of splitters.
	 * \param relative If position is scaled relatively when splitter is resized.
	 * \param border Clamping border, distance from extents in pixels.
	 */
	bool create(Widget* parent, const Point& position, bool relative, int border = 16);

	virtual void update(const Rect* rc = 0, bool immediate = false);
	
	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;
	
	virtual Size getMaximumSize() const;
	
	void setPosition(const Point& position);
	
	Point getPosition() const;
	
	void getWidgets(Ref< Widget > outWidgets[4]) const;
	
private:
	bool m_vertical;
	Point m_position;
	bool m_negativeX;
	bool m_negativeY;
	bool m_relative;
	int m_border;
	int m_drag;

	void setAbsolutePosition(const Point& position);

	Point getAbsolutePosition() const;

	void eventMouseMove(Event* event);
	
	void eventButtonDown(Event* event);
	
	void eventButtonUp(Event* event);
	
	void eventSize(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_QuadSplitter_H
