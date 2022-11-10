/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Event.h"
#include "Ui/EventSubject.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EventSubject", EventSubject, Object)

void EventSubject::raiseEvent(Event* event)
{
	T_ANONYMOUS_VAR(Ref< EventSubject >)(this);

	if (!event)
		return;

	const TypeInfo& eventType = type_of(event);
	for (auto i = m_eventHandlers.begin(); i != m_eventHandlers.end(); ++i)
	{
		if (!is_type_of(*i->first, eventType))
			continue;
		if (i->second.disableCounter != 0)
			continue;

		// Invoke event handlers reversed as the most prioritized are at the end and they should
		// be able to "consume" the event so it wont reach other, less prioritized, handlers.
		auto eventHandlers = i->second;
		for (int32_t j = (int32_t)eventHandlers.handlers.size() - 1; j >= 0; --j)
		{
			const auto& eventHandler = eventHandlers.handlers[j];
			for (EventHandlers::const_iterator j = eventHandler.begin(); j != eventHandler.end(); ++j)
			{
				Ref< IEventHandler > eventHandler = *j;
				if (eventHandler)
				{
					eventHandler->notify(event);
					if (event->consumed())
						break;
				}
			}
		}
	}
}

void EventSubject::removeAllEventHandlers()
{
	for (auto i = m_eventHandlers.begin(); i != m_eventHandlers.end(); ++i)
		i->second.handlers.clear();
}

void EventSubject::addEventHandler(const TypeInfo& eventType, IEventHandler* eventHandler)
{
	auto& eventHandlers = m_eventHandlers[&eventType];
	int32_t depth = 0;

	// Use class hierarchy depth as handler priority.
	for (const TypeInfo* type = getTypeInfo().getSuper(); type; type = type->getSuper())
		++depth;

	// Skip both Object and EventSubject bases as they will only take up space in the event vectors.
	depth -= 2;
	T_ASSERT(depth >= 0);

	// Ensure there are enough room in the event handlers vector.
	if (depth >= (int32_t)eventHandlers.handlers.size())
		eventHandlers.handlers.resize(depth + 1);

	// Insert event handler into vector.
	eventHandlers.handlers[depth].push_back(eventHandler);
}

void EventSubject::removeEventHandler(const TypeInfo& eventType, IEventHandler* eventHandler)
{
	auto& eventHandlers = m_eventHandlers[&eventType];
	for (auto i = eventHandlers.handlers.begin(); i != eventHandlers.handlers.end(); ++i)
		i->remove(eventHandler);
}

bool EventSubject::hasEventHandler(const TypeInfo& eventType)
{
	return !m_eventHandlers[&eventType].handlers.empty();
}

void EventSubject::enableEventHandlers(const TypeInfo& eventType)
{
	auto& eventHandlers = m_eventHandlers[&eventType];
	eventHandlers.disableCounter--;
}

void EventSubject::disableEventHandlers(const TypeInfo& eventType)
{
	auto& eventHandlers = m_eventHandlers[&eventType];
	eventHandlers.disableCounter++;
}

	}
}
