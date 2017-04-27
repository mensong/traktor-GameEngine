/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/TreeView/TreeViewContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewContentChangeEvent", TreeViewContentChangeEvent, ContentChangeEvent)

TreeViewContentChangeEvent::TreeViewContentChangeEvent(EventSubject* sender, TreeViewItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

TreeViewItem* TreeViewContentChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
