#include "Flash/Action/ActionDate.h"
#include "Flash/Action/Classes/AsDate.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionDate", ActionDate, ActionObject)

ActionDate::ActionDate()
:	ActionObject(AsDate::getInstance())
{
}

	}
}
