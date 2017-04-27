/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_TrapezoidShadowProjection_H
#define traktor_world_TrapezoidShadowProjection_H

#include "World/IWorldShadowProjection.h"

namespace traktor
{
	namespace world
	{

class TrapezoidShadowProjection : public IWorldShadowProjection
{
	T_RTTI_CLASS;

public:
	virtual void calculate(
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		const Aabb3& shadowBox,
		float shadowFarZ,
		bool quantizeProjection,
		Matrix44& outLightView,
		Matrix44& outLightProjection,
		Frustum& outShadowFrustum
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_world_TrapezoidShadowProjection_H
