#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/Entity.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Simple/WorldRendererSimple.h"
#include "World/Simple/WorldRenderPassSimple.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererSimple", 0, WorldRendererSimple, IWorldRenderer)

bool WorldRendererSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;
	return true;
}

void WorldRendererSimple::destroy()
{
	m_entityRenderers = nullptr;
}

void WorldRendererSimple::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->setOutput(outputTargetSetId);
	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			// Default visual parameters.
			auto globalProgramParams = renderContext->alloc< render::ProgramParameters >();
			globalProgramParams->beginParameters(renderContext);
			globalProgramParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			globalProgramParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			globalProgramParams->endParameters(renderContext);

			// Build visual context.
			WorldRenderPassSimple defaultPass(
				s_techniqueSimpleColor,
				globalProgramParams,
				worldRenderView.getView()
			);

			wc.build(worldRenderView, defaultPass, rootEntity);
			wc.flush(worldRenderView, defaultPass);
			renderContext->merge(render::RpAll);
		}
	);
	renderGraph.addPass(rp);
}

	}
}
