/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/World/Vehicle/Wheel.h"
#include "Physics/World/Vehicle/WheelData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Wheel", Wheel, Object)

Wheel::Wheel(const WheelData* data)
:	data(data)
,	angle(0.0f)
,	velocity(0.0f)
,	direction(0.0f, 0.0f, 1.0f)
,	directionPerp(1.0f, 0.0f, 0.0f)
,	suspensionLength(data->getSuspensionLength().max)
,	contact(false)
,	contactFudge(0.0f)
,	contactMaterial(0)
,	contactPosition(Vector4::origo())
,	contactNormal(Vector4::zero())
,	contactVelocity(Vector4::zero())
,	sliding(false)
{
}

	}
}
