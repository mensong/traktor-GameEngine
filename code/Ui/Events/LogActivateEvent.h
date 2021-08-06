#pragma once

#include "Core/Guid.h"
#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! Log symbol activated.
 * \ingroup UI
 */
class T_DLLCLASS LogActivateEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit LogActivateEvent(EventSubject* sender, const Guid& symbolId);

	const Guid& getSymbolId() const;

private:
	Guid m_symbolId;
};

	}
}
