/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Remote/Messages/DbmWriteData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmWriteData", 0, DbmWriteData, IMessage)

DbmWriteData::DbmWriteData(uint32_t handle, const std::wstring& name)
:	m_handle(handle)
,	m_name(name)
{
}

void DbmWriteData::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
