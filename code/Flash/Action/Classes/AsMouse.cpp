#include <algorithm>
#include "Flash/Action/Classes/AsMouse.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMouse", AsMouse, ActionClass)

Ref< AsMouse > AsMouse::createInstance()
{
	Ref< AsMouse > instance = new AsMouse();
	instance->createPrototype();
	instance->setReadOnly();
	return instance;
}

AsMouse::AsMouse()
:	ActionClass(L"Mouse")
{
}

void AsMouse::eventMouseDown(ActionVM* actionVM, ActionContext* context, int x, int y, int button)
{
	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onButtonDown", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(actionVM, &callerFrame, (*i));
		}
	}
}

void AsMouse::eventMouseUp(ActionVM* actionVM, ActionContext* context, int x, int y, int button)
{
	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onButtonUp", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(actionVM, &callerFrame, (*i));
		}
	}
}

void AsMouse::eventMouseMove(ActionVM* actionVM, ActionContext* context, int x, int y, int button)
{
	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onMove", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(actionVM, &callerFrame, (*i));
		}
	}
}

void AsMouse::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"addListener", createNativeFunctionValue(this, &AsMouse::Mouse_addListener));
	prototype->setMember(L"removeListener", createNativeFunctionValue(this, &AsMouse::Mouse_removeListener));
	prototype->setMember(L"show", createNativeFunctionValue(this, &AsMouse::Mouse_show));
	prototype->setMember(L"hide", createNativeFunctionValue(this, &AsMouse::Mouse_hide));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsMouse::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsMouse::Mouse_addListener(CallArgs& ca)
{
	m_listeners.push_back(ca.args[0].getObjectSafe());
}

void AsMouse::Mouse_removeListener(CallArgs& ca)
{
	RefArray< ActionObject >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), ca.args[0].getObjectSafe());
	if (i != m_listeners.end())
		m_listeners.erase(i);
}

void AsMouse::Mouse_show(CallArgs& ca)
{
}

void AsMouse::Mouse_hide(CallArgs& ca)
{
}

	}
}
