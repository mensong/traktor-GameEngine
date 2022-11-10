/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Physics/JointDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Ball joint description.
 * \ingroup Physics
 */
class T_DLLCLASS BallJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	BallJointDesc();

	void setAnchor(const Vector4& anchor);

	const Vector4& getAnchor() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_anchor;
};

	}
}

