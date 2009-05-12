#include "Ui/Events/ShowEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShowEvent", ShowEvent, Event)

ShowEvent::ShowEvent(EventSubject* sender, Object* item, bool visible)
:	Event(sender, item)
,	m_visible(visible)
{
}

bool ShowEvent::isVisible() const
{
	return m_visible;
}

	}
}
