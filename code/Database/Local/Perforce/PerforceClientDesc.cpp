/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Database/Local/Perforce/PerforceClientDesc.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.PerforceClientDesc", 0, PerforceClientDesc, ISerializable)

void PerforceClientDesc::serialize(ISerializer& s)
{
	const MemberEnum< SecurityLevel >::Key c_SecurityLevel_Keys[] =
	{
		{ L"SlLow", SlLow },
		{ L"SlHigh", SlHigh },
		0
	};

	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< std::wstring >(L"port", m_port);
	s >> Member< std::wstring >(L"user", m_user);
	s >> Member< std::wstring >(L"password", m_password);
	s >> Member< std::wstring >(L"client", m_client);
	s >> MemberEnum< SecurityLevel >(L"securityLevel", m_securityLevel, c_SecurityLevel_Keys);
}

	}
}
