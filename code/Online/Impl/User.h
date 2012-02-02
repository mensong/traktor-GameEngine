#ifndef traktor_online_User_H
#define traktor_online_User_H

#include "Online/IUser.h"

namespace traktor
{
	namespace online
	{

class IUserProvider;

class User : public IUser
{
	T_RTTI_CLASS;

public:
	virtual bool getName(std::wstring& outName) const;

	virtual bool setMetaValue(const std::wstring& key, const std::wstring& value);

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const;

	virtual bool sendP2PData(const void* data, size_t size, bool reliable);

private:
	friend class UserCache;

	Ref< IUserProvider > m_userProvider;
	uint64_t m_handle;

	User(IUserProvider* userProvider, uint64_t handle);
};

	}
}

#endif	// traktor_online_User_H
