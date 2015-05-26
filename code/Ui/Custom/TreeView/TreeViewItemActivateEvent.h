#ifndef traktor_ui_custom_TreeViewItemActivateEvent_H
#define traktor_ui_custom_TreeViewItemActivateEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class TreeViewItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS TreeViewItemActivateEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	TreeViewItemActivateEvent(EventSubject* sender, TreeViewItem* item);

	TreeViewItem* getItem() const;

private:
	Ref< TreeViewItem > m_item;
};
	
		}
	}
}

#endif	// traktor_ui_custom_TreeViewItemActivateEvent_H
