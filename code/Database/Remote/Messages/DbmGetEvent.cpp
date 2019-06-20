#include "Database/Remote/Messages/DbmGetEvent.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetEvent", 0, DbmGetEvent, IMessage)

DbmGetEvent::DbmGetEvent(uint32_t handle, uint64_t sqnr)
:	m_handle(handle)
,	m_sqnr(sqnr)
{
}

void DbmGetEvent::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< uint64_t >(L"m_sqnr", m_sqnr);
}

	}
}
