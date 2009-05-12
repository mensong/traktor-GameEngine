#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/Classes/AsFunction.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunctionNative", ActionFunctionNative, ActionFunction)

ActionFunctionNative::ActionFunctionNative(CallFnc* callFnc)
:	ActionFunction(L"<native>")
,	m_callFnc(callFnc)
{
	// Do this inside constructor to prevent infinite recursion.
	ActionValue classPrototype;
	if (AsFunction::getInstance()->getLocalMember(L"prototype", classPrototype))
		setMember(L"__proto__", classPrototype);
}

ActionValue ActionFunctionNative::call(ActionVM* vm, ActionFrame* callerFrame, ActionObject* self)
{
	ActionValueStack& callerStack = callerFrame->getStack();
	int argCount = !callerStack.empty() ? int(callerStack.pop().getNumber()) : 0;

	CallArgs fnc;
	fnc.vm = vm;
	fnc.context = callerFrame->getContext();
	fnc.self = self;
	fnc.args.resize(argCount);

	for (int i = 0; i < argCount; ++i)
		fnc.args[i] = callerStack.pop();

	if (m_callFnc)
		m_callFnc->call(fnc);

	return fnc.ret;
}

	}
}
