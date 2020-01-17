#pragma once

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! Component entity renderer.
 * \ingroup World
 */
class T_DLLCLASS ComponentEntityRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const Object* renderable,
		AlignedVector< world::Light >& outLights
	) override final;

	virtual void build(
		WorldContext& worldContext,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		WorldContext& worldContext,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

	virtual void flush(WorldContext& worldContext) override final;
};

	}
}

