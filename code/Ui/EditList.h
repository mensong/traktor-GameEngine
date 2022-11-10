/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_ui_EditList_H
#define traktor_ui_EditList_H

#include "Ui/Edit.h"
#include "Ui/ListBox/ListBox.h"

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

class T_DLLCLASS EditList : public traktor::ui::ListBox
{
	T_RTTI_CLASS;

public:
	enum WidgetStyles
	{
		WsAutoAdd = (WsUser << 3),
		WsAutoRemove = (WsUser << 4)
	};

	EditList();

	bool create(Widget* parent, int32_t style = traktor::ui::ListBox::WsDefault);

private:
	Ref< Edit > m_editItem;
	int32_t m_editId;
	bool m_autoAdd;
	bool m_autoRemove;

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventEditFocus(FocusEvent* event);
};

	}
}

#endif	// EditList_H
