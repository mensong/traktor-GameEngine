#include "Online/Psn/SessionPsn.h"
#include "Online/Psn/SessionManagerPsn.h"
#include "Online/Psn/UserPsn.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerPsn", 0, SessionManagerPsn, ISessionManager)

SessionManagerPsn::SessionManagerPsn()
:	m_user(new UserPsn(L"N/A"))
{
}

bool SessionManagerPsn::create()
{
	return true;
}

void SessionManagerPsn::destroy()
{
}

bool SessionManagerPsn::getAvailableUsers(RefArray< IUser >& outUsers)
{
	return true;
}

Ref< IUser > SessionManagerPsn::getCurrentUser()
{
	return m_user;
}

Ref< ISession > SessionManagerPsn::createSession(IUser* user)
{
	return new SessionPsn(checked_type_cast< UserPsn*, false >(user));
}

	}
}
