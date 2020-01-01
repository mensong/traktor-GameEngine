#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace world
	{

class Entity;
class IEntityRenderer;
class IWorldRenderPass;
class WorldEntityRenderers;
class WorldRenderView;

/*! World render thread context.
 * \ingroup World
 */
class T_DLLCLASS WorldContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit WorldContext(WorldEntityRenderers* entityRenderers, render::RenderContext* renderContext);

	void build(WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Object* renderable);

	void flush(WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Entity* rootEntity);

	WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	render::RenderContext* getRenderContext() const { return m_renderContext; }

private:
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::RenderContext > m_renderContext;
	const TypeInfo* m_lastRenderableType;
	IEntityRenderer* m_lastRenderer;
};

	}
}

