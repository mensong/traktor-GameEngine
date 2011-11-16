#ifndef traktor_world_TrapezoidShadowProjection_H
#define traktor_world_TrapezoidShadowProjection_H

#include "World/IWorldShadowProjection.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;

class TrapezoidShadowProjection : public IWorldShadowProjection
{
	T_RTTI_CLASS;

public:
	TrapezoidShadowProjection(const WorldRenderSettings& settings);

	virtual void calculate(
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		const Aabb3& shadowBox,
		Matrix44& outLightView,
		Matrix44& outLightProjection,
		Matrix44& outLightSquareProjection,
		Frustum& outShadowFrustum
	) const;

private:
	WorldRenderSettings m_settings;
};

	}
}

#endif	// traktor_world_TrapezoidShadowProjection_H
