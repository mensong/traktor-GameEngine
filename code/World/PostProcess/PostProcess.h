#ifndef traktor_world_PostProcess_H
#define traktor_world_PostProcess_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class RenderTargetSet;
class ScreenRenderer;

	}

	namespace world
	{

class PostProcessSettings;
class WorldRenderView;

/*! \brief Frame buffer post processing system.
 * \ingroup World
 *
 * Predefined targets:
 * 0 - Frame buffer, write only.
 * 1 - Source color buffer, read only.
 * 2 - Source depth buffer, read only.
 */
class T_DLLCLASS PostProcess : public Object
{
	T_RTTI_CLASS(PostProcess)

public:
	bool create(
		PostProcessSettings* settings,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	bool render(
		const WorldRenderView& worldRenderView,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		render::RenderTargetSet* frameBuffer,
		render::RenderTargetSet* depthBuffer,
		float deltaTime
	);

	void setTarget(render::IRenderView* renderView, uint32_t id);

	Ref< render::RenderTargetSet >& getTargetRef(uint32_t id);

	const std::map< uint32_t, Ref< render::RenderTargetSet > >& getTargets() const;

	render::RenderTargetSet* createOutputTarget(
		render::IRenderSystem* renderSystem,
		int32_t width,
		int32_t height,
		int32_t multiSample
	);

private:
	Ref< PostProcessSettings > m_settings;
	Ref< render::ScreenRenderer > m_screenRenderer;
	std::map< uint32_t, Ref< render::RenderTargetSet > > m_targets;
	Ref< render::RenderTargetSet > m_currentTarget;
	int32_t m_definedWidth;
	int32_t m_definedHeight;
};

	}
}

#endif	// traktor_world_PostProcess_H
