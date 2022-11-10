/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/ShapeDesc.h"
#include "Core/Math/Vector4.h"

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

/*! Box collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS BoxShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	BoxShapeDesc();

	void setExtent(const Vector4& extent);

	const Vector4& getExtent() const;

	void setMargin(float margin);

	float getMargin() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_extent;
	float m_margin;
};

	}
}

