#include "World/IEntityComponent.h"
#include "World/WorldContext.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntityRenderer", ComponentEntityRenderer, IEntityRenderer)

const TypeInfoSet ComponentEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ComponentEntity >();
}

void ComponentEntityRenderer::gather(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void ComponentEntityRenderer::build(
	WorldContext& worldContext,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ComponentEntity* componentEntity = mandatory_non_null_type_cast< ComponentEntity* >(renderable);
	for (auto component : componentEntity->getComponents())
		worldContext.build(worldRenderView, worldRenderPass, component);
}

void ComponentEntityRenderer::flush(
	WorldContext& worldContext,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void ComponentEntityRenderer::flush(WorldContext& worldContext)
{
}

	}
}
