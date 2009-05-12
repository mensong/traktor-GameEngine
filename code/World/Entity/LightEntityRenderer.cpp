#include <limits>
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/PointLightEntity.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightEntityRenderer", LightEntityRenderer, EntityRenderer)

const TypeSet LightEntityRenderer::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< DirectionalLightEntity >());
	typeSet.insert(&type_of< PointLightEntity >());
	return typeSet;
}

void LightEntityRenderer::render(
	WorldContext* worldContext,
	WorldRenderView* worldRenderView,
	Entity* entity
)
{
	WorldRenderView::Light light;

	// Note: Even though we modify the light here the shadow map isn't affected until next frame.
	if (DirectionalLightEntity* directionalLightEntity = dynamic_type_cast< DirectionalLightEntity* >(entity))
	{
		Matrix44 transform;
		directionalLightEntity->getTransform(transform);

		light.type = WorldRenderView::LtDirectional;
		light.position = transform.translation();
		light.direction = transform.axisY();
		light.sunColor = directionalLightEntity->getSunColor();
		light.baseColor = directionalLightEntity->getBaseColor();
		light.shadowColor = directionalLightEntity->getShadowColor();
		light.range = Scalar(0.0f);

		worldRenderView->addLight(light);
	}
	else if (PointLightEntity* pointLightEntity = dynamic_type_cast< PointLightEntity* >(entity))
	{
		Matrix44 transform;
		pointLightEntity->getTransform(transform);

		Vector4 center = worldRenderView->getView() * transform.translation();
		if (!worldRenderView->getCullFrustum().inside(center, Scalar(pointLightEntity->getRange())))
			return;

		light.type = WorldRenderView::LtPoint;
		light.position = transform.translation();
		light.direction = Vector4::zero();
		light.sunColor = pointLightEntity->getSunColor();
		light.baseColor = pointLightEntity->getBaseColor();
		light.shadowColor = pointLightEntity->getShadowColor();
		light.range = Scalar(pointLightEntity->getRange());

		worldRenderView->addLight(light);
	}
}

void LightEntityRenderer::flush(
	WorldContext* worldContext,
	WorldRenderView* worldRenderView
)
{
}

	}
}
