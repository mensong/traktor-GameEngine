/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

	namespace render
	{

class PrimitiveRenderer;

	}

	namespace physics
	{

class JointDesc;

class IPhysicsJointRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const = 0;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform jointTransform[2],
		const Transform body1Transform[2],
		const Transform body2Transform[2],
		const JointDesc* jointDesc
	) const = 0;
};

	}
}

