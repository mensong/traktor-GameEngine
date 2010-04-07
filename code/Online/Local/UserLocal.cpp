#include "Online/Local/UserLocal.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserLocal", UserLocal, IUser)

UserLocal::UserLocal(sql::IConnection* db, int32_t id, const std::wstring& name)
:	m_db(db)
,	m_id(id)
,	m_name(name)
{
}

std::wstring UserLocal::getName() const
{
	return m_name;
}

bool UserLocal::getFriends(RefArray< IUser >& outFriends) const
{
	Ref< sql::IResultSet > rs = m_db->executeQuery(L"select * from Users where id = (select friendId from Friends where userId = " + toString(m_id) + L")");
	if (!rs)
		return false;

	while (rs->next())
	{
		outFriends.push_back(new UserLocal(
			m_db,
			rs->getInt32(L"id"),
			rs->getString(L"name")
		));
	}

	return true;
}

bool UserLocal::sendMessage(const std::wstring& message) const
{
	return false;
}

	}
}
