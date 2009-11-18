#ifndef traktor_spray_EffectEntityRenderer_H
#define traktor_spray_EffectEntityRenderer_H

#include "World/Entity/IEntityRenderer.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace spray
	{

class PointRenderer;
class EffectEntity;

/*! \brief Effect entity renderer.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EffectEntityRenderer(render::IRenderSystem* renderSystem, float cullNearDistance, float fadeNearRange);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	);

private:
	Ref< PointRenderer > m_pointRenderer;
	render::handle_t m_defaltTechnique;
};

	}
}

#endif	// traktor_spray_EffectEntityRenderer_H
