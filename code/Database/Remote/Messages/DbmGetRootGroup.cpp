#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetRootGroup", DbmGetRootGroup, IMessage)

bool DbmGetRootGroup::serialize(ISerializer& s)
{
	return true;
}

	}
}
