/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/LightComponent.h"

namespace traktor::weather
{
	namespace
	{

const resource::Id< render::Shader > c_shaderClouds2D(Guid(L"{9F52BE0A-0C1A-4928-91D9-9D32296CB8F3}"));
const resource::Id< render::Shader > c_shaderClouds3D(Guid(L"{EF88CE37-0917-4402-B2D1-6E3F2D3CCCCF}"));

const render::Handle s_handleWeather_SkyRadius(L"Weather_SkyRadius");
const render::Handle s_handleWeather_SkyRotation(L"Weather_SkyRotation");
const render::Handle s_handleWeather_SkyTexture(L"Weather_SkyTexture");
const render::Handle s_handleWeather_SkyCloud2D(L"Weather_SkyCloud2D");
const render::Handle s_handleWeather_SkyCloud3D(L"Weather_SkyCloud3D");
const render::Handle s_handleWeather_SkyIntensity(L"Weather_SkyIntensity");
const render::Handle s_handleWeather_SkySunDirection(L"Weather_SkySunDirection");
const render::Handle s_handleWeather_SkySunColor(L"Weather_SkySunColor");
const render::Handle s_handleWeather_SkyEyePosition(L"Weather_SkyEyePosition");

const int32_t c_longitudes = 16;
const int32_t c_latitudes = 24;
const int32_t c_vertexCount = (c_longitudes + 1) * c_latitudes;
const int32_t c_triangleCount = ((c_latitudes - 1) * ((c_longitudes + 1) * 2));
const int32_t c_indexCount = c_triangleCount * 3;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyComponent", SkyComponent, world::IEntityComponent)

SkyComponent::SkyComponent(
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< render::ITexture >& texture,
	float intensity
)
:	m_shader(shader)
,	m_texture(texture)
,	m_transform(Transform::identity())
,	m_intensity(intensity)
{
}

bool SkyComponent::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, 0));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(
		render::BuVertex,
		c_vertexCount,
		sizeof(float) * 2,
		false
	);
	if (!m_vertexBuffer)
		return false;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	for (int32_t i = 0; i < c_latitudes; ++i)
	{
		const float phi = float(i) / (c_latitudes - 1);
		for (int32_t j = 0; j <= c_longitudes; ++j)
		{
			const float theta = float(j) / c_longitudes;
			*vertex++ = phi;
			*vertex++ = theta;
		}
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(
		render::BuIndex,
		c_indexCount,
		sizeof(uint16_t),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	if (!index)
		return false;

	for (int32_t k = 0; k < c_latitudes - 1; ++k)
	{
		const int32_t o = k * (c_longitudes + 1);
		for (int32_t i = 0, j = c_longitudes; i <= c_longitudes; j = i, ++i)
		{
			*index++ = o + i;
			*index++ = o + i + c_longitudes + 1;
			*index++ = o + j;
			*index++ = o + j;
			*index++ = o + i + c_longitudes + 1;
			*index++ = o + j + c_longitudes + 1;
		}
	}

	m_indexBuffer->unlock();

	m_primitives = render::Primitives(
		render::PrimitiveType::Triangles,
		0,
		c_triangleCount,
		0,
		c_vertexCount - 1
	);

	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 512;
	stcd.height = 512;
	stcd.mipCount = 1;
	stcd.format = render::TfR32G32B32A32F;
	stcd.shaderStorage = true;
	m_cloudTextures[0] = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);

	render::VolumeTextureCreateDesc vtcd = {};
	vtcd.width = 64;
	vtcd.height = 64;
	vtcd.depth = 64;
	vtcd.mipCount = 1;
	vtcd.format = render::TfR32G32B32A32F;
	vtcd.shaderStorage = true;
	m_cloudTextures[1] = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W);

	if (!m_cloudTextures[0] || !m_cloudTextures[1])
		return false;

	if (!resourceManager->bind(c_shaderClouds2D, m_shaderClouds2D))
		return false;
	if (!resourceManager->bind(c_shaderClouds3D, m_shaderClouds3D))
		return false;

	return true;
}

SkyComponent::~SkyComponent()
{
	destroy();
}

void SkyComponent::destroy()
{
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_cloudTextures[0]);
	safeDestroy(m_cloudTextures[1]);
	m_shader.clear();
	m_texture.clear();
	m_shaderClouds2D.clear();
	m_shaderClouds3D.clear();
}

void SkyComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void SkyComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 SkyComponent::getBoundingBox() const
{
	const float c_radius = 1e4f;
	return Aabb3(Vector4(-c_radius, -c_radius, -c_radius, 1.0f), Vector4(c_radius, c_radius, c_radius, 1.0f));
}

void SkyComponent::update(const world::UpdateParams& update)
{
}

void SkyComponent::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView
)
{
	const static render::Handle s_handleOutputTexture(L"Weather_OutputTexture");

	if (m_shaderClouds2D.changed() || m_shaderClouds3D.changed())
	{
		m_dirty = true;
		m_shaderClouds2D.consume();
		m_shaderClouds3D.consume();
	}

	if (m_dirty)
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Sky compute clouds");
		rp->addBuild([=](const render::RenderGraph&, render::RenderContext* renderContext) {
			{
				auto renderBlock = renderContext->alloc< render::ComputeRenderBlock >(L"Sky clouds 2D");
				renderBlock->program = m_shaderClouds2D->getProgram().program;
				renderBlock->workSize[0] = 512;
				renderBlock->workSize[1] = 512;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setImageViewParameter(s_handleOutputTexture, m_cloudTextures[0]);
				renderBlock->programParams->endParameters(renderContext);

				renderContext->enqueue(renderBlock);
			}
			{
				auto renderBlock = renderContext->alloc< render::ComputeRenderBlock >(L"Sky clouds 3D");
				renderBlock->program = m_shaderClouds3D->getProgram().program;
				renderBlock->workSize[0] = 64;
				renderBlock->workSize[1] = 64;
				renderBlock->workSize[2] = 64;

				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setImageViewParameter(s_handleOutputTexture, m_cloudTextures[1]);
				renderBlock->programParams->endParameters(renderContext);

				renderContext->enqueue(renderBlock);
			}
		});
		context.getRenderGraph().addPass(rp);

		m_dirty = false;
	}
}

void SkyComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	// Get sun from directional light in same entity as sky component.
	Vector4 sunDirection = m_transform.axisY();
	Vector4 sunColor = Vector4(1.0f, 0.9f, 0.85f) * 1.8_simd;
	if (m_owner != nullptr)
	{
		auto lightComponent = m_owner->getComponent< world::LightComponent >();
		sunColor = lightComponent->getColor();
	}

	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const float rotation = m_transform.rotation().toEulerAngles().x();

	auto renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"Sky");

	// Render sky after all opaques but before of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt16;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyRadius, worldRenderView.getViewFrustum().getFarZ() - 10.0f);
	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyRotation, rotation);
	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyIntensity, m_intensity);

	renderBlock->programParams->setVectorParameter(s_handleWeather_SkySunDirection, sunDirection);
	renderBlock->programParams->setVectorParameter(s_handleWeather_SkySunColor, sunColor);
	renderBlock->programParams->setVectorParameter(s_handleWeather_SkyEyePosition, eyePosition);

	renderBlock->programParams->setTextureParameter(s_handleWeather_SkyTexture, m_texture);
	renderBlock->programParams->setTextureParameter(s_handleWeather_SkyCloud2D, m_cloudTextures[0]);
	renderBlock->programParams->setTextureParameter(s_handleWeather_SkyCloud3D, m_cloudTextures[1]);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

}
