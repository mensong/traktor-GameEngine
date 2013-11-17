#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepGodRay.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepGodRay", 0, PostProcessStepGodRay, PostProcessStep)

PostProcessStepGodRay::PostProcessStepGodRay()
:	m_lightDistance(100.0f)
{
}

Ref< PostProcessStep::Instance > PostProcessStepGodRay::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceGodRay::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	return new InstanceGodRay(this, shader, sources);
}

void PostProcessStepGodRay::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> Member< float >(L"lightDistance", m_lightDistance);
}

PostProcessStepGodRay::Source::Source()
:	index(0)
{
}

void PostProcessStepGodRay::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

PostProcessStepGodRay::InstanceGodRay::InstanceGodRay(const PostProcessStepGodRay* step, const resource::Proxy< render::Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = render::getParameterHandle(L"Time");
	m_handleDeltaTime = render::getParameterHandle(L"DeltaTime");
	m_handleAlpha = render::getParameterHandle(L"Alpha");
	m_handleScreenLightPosition = render::getParameterHandle(L"ScreenLightPosition");
}

void PostProcessStepGodRay::InstanceGodRay::destroy()
{
}

void PostProcessStepGodRay::InstanceGodRay::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	Vector4 lightPositionView = params.view * (params.godRayDirection * Scalar(m_step->m_lightDistance)).xyz1();
	Vector4 lightPosition = params.projection * lightPositionView;
	
	// If light is behind camera near plane then skip god rays; but
	// ensure target is cleared so no lingering rays are kept.
	if (params.godRayDirection.length2() <= FUZZY_EPSILON || lightPosition.w() <= 0.0f)
	{
		const static Color4f c_clearColor(0.0f, 0.0f, 0.0f, 0.0f);
		renderView->clear(render::CfColor, &c_clearColor, 0.0f, 0);
		return;
	}

	lightPosition /= lightPosition.w();

	postProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	m_shader->setFloatParameter(m_handleAlpha, abs(lightPositionView.xyz0().normalized().z()));
	m_shader->setVectorParameter(m_handleScreenLightPosition, lightPosition * Vector4(1.0f, -1.0f, 0.0f, 0.0f));

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		render::RenderTargetSet* source = postProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	screenRenderer->draw(renderView, m_shader);

	m_time += params.deltaTime;
}

	}
}
