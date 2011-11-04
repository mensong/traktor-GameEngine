#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldRenderView.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepBlur.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepBlur", 1, PostProcessStepBlur, PostProcessStep)

PostProcessStepBlur::PostProcessStepBlur()
:	m_taps(15)
{
}

Ref< PostProcessStepBlur::Instance > PostProcessStepBlur::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< render::Shader > shader = m_shader;
	if (!resourceManager->bind(shader))
		return 0;

	std::vector< InstanceBlur::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].paramSize = render::getParameterHandle(m_sources[i].param + L"_Size");
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	AlignedVector< Vector4 > gaussianOffsetWeights(m_taps);
	float totalWeight = 0.0f;

	float step = 1.0f / (m_taps - 1.0f);
	float angleMin = (PI * step) / 2.0f;
	float angleMax = PI - angleMin;

	for (int i = 0; i < m_taps; ++i)
	{
		float phi = (float(i) * step) * (angleMax - angleMin) + angleMin;

		float weight = sinf(phi);
		totalWeight += weight;

		gaussianOffsetWeights[i].set(
			i - m_taps / 2.0f,
			weight,
			0.0f,
			0.0f
		);
	}

	Vector4 invWeight(1.0f, 1.0f / totalWeight, 1.0f, 1.0f);
	for (int i = 0; i < m_taps; ++i)
		gaussianOffsetWeights[i] *= invWeight;

	return new InstanceBlur(
		shader,
		sources,
		m_direction,
		gaussianOffsetWeights
	);
}

bool PostProcessStepBlur::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> Member< Vector4 >(L"direction", m_direction, AttributeDirection());
	if (s.getVersion() >= 1)
		s >> Member< int32_t >(L"taps", m_taps);
	return true;
}

PostProcessStepBlur::Source::Source()
:	index(0)
{
}

bool PostProcessStepBlur::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

// Instance

PostProcessStepBlur::InstanceBlur::InstanceBlur(
	const resource::Proxy< render::Shader >& shader,
	const std::vector< Source >& sources,
	const Vector4& direction,
	const AlignedVector< Vector4 >& gaussianOffsetWeights
)
:	m_shader(shader)
,	m_sources(sources)
,	m_direction(direction)
,	m_gaussianOffsetWeights(gaussianOffsetWeights)
{
	m_handleGaussianOffsetWeights = render::getParameterHandle(L"GaussianOffsetWeights");
	m_handleDirection = render::getParameterHandle(L"Direction");
	m_handleViewFar = render::getParameterHandle(L"ViewFar");
	m_handleDepthRange = render::getParameterHandle(L"DepthRange");
}

void PostProcessStepBlur::InstanceBlur::destroy()
{
}

void PostProcessStepBlur::InstanceBlur::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	if (!m_shader.validate())
		return;

	postProcess->prepareShader(m_shader);

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		Ref< render::RenderTargetSet > source = postProcess->getTargetRef(i->source);
		if (source)
		{
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
			m_shader->setVectorParameter(i->paramSize, Vector4(
				float(source->getWidth()),
				float(source->getHeight()),
				0.0f,
				0.0f
			));
		}
	}

	m_shader->setVectorArrayParameter(m_handleGaussianOffsetWeights, &m_gaussianOffsetWeights[0], m_gaussianOffsetWeights.size());
	m_shader->setVectorParameter(m_handleDirection, m_direction);
	m_shader->setFloatParameter(m_handleViewFar, params.viewFrustum.getFarZ());
	m_shader->setFloatParameter(m_handleDepthRange, params.depthRange);

	screenRenderer->draw(renderView, m_shader);
}

	}
}
