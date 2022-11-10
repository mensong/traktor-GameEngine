/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/MoveEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MoveEvent", MoveEvent, Event)

MoveEvent::MoveEvent(EventSubject* sender, const Point& position)
:	Event(sender)
,	m_position(position)
{
}

const Point& MoveEvent::getPosition() const
{
	return m_position;
}

	}
}
