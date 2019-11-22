#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/PropertyList/PropertyList.h"
#include "Ui/PropertyList/StaticPropertyItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StaticPropertyItem", StaticPropertyItem, PropertyItem)

StaticPropertyItem::StaticPropertyItem(const std::wstring& text, const std::wstring& value) :
	PropertyItem(text),
	m_value(value)
{
}

void StaticPropertyItem::setValue(const std::wstring& value)
{
	m_value = value;
}

const std::wstring& StaticPropertyItem::getValue() const
{
	return m_value;
}

void StaticPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	canvas.drawText(rc.inflate(-2, 0), m_value, AnLeft, AnCenter);
}

bool StaticPropertyItem::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (clipboard)
		return clipboard->setText(m_value);
	else
		return false;
}

	}
}
