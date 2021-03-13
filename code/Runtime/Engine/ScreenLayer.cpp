#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/ScreenLayer.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPass.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ScreenLayer", ScreenLayer, Layer)

ScreenLayer::ScreenLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< render::Shader >& shader
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_shader(shader)
{
}

void ScreenLayer::destroy()
{
	safeDestroy(m_screenRenderer);
	Layer::destroy();
}

void ScreenLayer::transition(Layer* fromLayer)
{
	bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (!permit)
		return;

	ScreenLayer* fromScreenLayer = mandatory_non_null_type_cast< ScreenLayer* >(fromLayer);
	m_screenRenderer = fromScreenLayer->m_screenRenderer;
	fromScreenLayer->m_screenRenderer = nullptr;
}

void ScreenLayer::prepare(const UpdateInfo& info)
{
}

void ScreenLayer::update(const UpdateInfo& info)
{
}

void ScreenLayer::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
	if (!m_shader)
		return;

	if (!m_screenRenderer)
	{
		m_screenRenderer = new render::ScreenRenderer();
		m_screenRenderer->create(m_environment->getRender()->getRenderSystem());
	}

	Ref< render::RenderPass > rp = new render::RenderPass(L"Screen");
	rp->setOutput(0, render::TfAll, render::TfAll);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
		auto programParams = renderContext->alloc< render::ProgramParameters >();
		programParams->beginParameters(renderContext);
		if (m_parameterCallback)
			m_parameterCallback(programParams);
		programParams->endParameters(renderContext);
		m_screenRenderer->draw(renderContext, m_shader, programParams);
	});
	renderGraph.addPass(rp);
}

void ScreenLayer::preReconfigured()
{
}

void ScreenLayer::postReconfigured()
{
}

void ScreenLayer::suspend()
{
}

void ScreenLayer::resume()
{
}

	}
}
