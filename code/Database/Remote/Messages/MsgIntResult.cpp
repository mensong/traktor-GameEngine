/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Remote/Messages/MsgIntResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgIntResult", 0, MsgIntResult, IMessage)

MsgIntResult::MsgIntResult(int32_t value)
:	m_value(value)
{
}

void MsgIntResult::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"value", m_value);
}

	}
}
