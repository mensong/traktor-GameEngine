#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/IrradianceGrid.h"
#include "World/Deferred/LightRendererDeferred.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const resource::Id< render::Shader > c_lightShader(Guid(L"{707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}"));
const resource::Id< render::Shader > c_reflectionShader(Guid(L"{F04EEA34-85E0-974F-BE97-79D24C6ACFBD}"));
const resource::Id< render::Shader > c_fogShader(Guid(L"{9453D74C-76C4-8748-9A5B-9E3D6D4F9406}"));

render::handle_t s_handleTime;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleReflectionsEnable;
render::handle_t s_handleIrradianceEnable;
render::handle_t s_handleExtent;
render::handle_t s_handleProjection;
render::handle_t s_handleView;
render::handle_t s_handleViewInverse;
render::handle_t s_handleMagicCoeffs;
render::handle_t s_handleScreenMap;
render::handle_t s_handleDepthMap;
render::handle_t s_handleNormalMap;
render::handle_t s_handleMiscMap;
render::handle_t s_handleColorMap;
render::handle_t s_handleOcclusionMap;
render::handle_t s_handleLightDiffuseMap;
render::handle_t s_handleLightSpecularMap;
render::handle_t s_handleProbeDiffuse;
render::handle_t s_handleProbeSpecular;
render::handle_t s_handleProbeSpecularMips;
render::handle_t s_handleFogDistanceAndDensity;
render::handle_t s_handleFogColor;
render::handle_t s_handleLightCount;
render::handle_t s_handleShadowMask;
render::handle_t s_handleShadowMapAtlas;
render::handle_t s_handleReflectionMap;
render::handle_t s_handleLightSBuffer;
render::handle_t s_handleTileSBuffer;
render::handle_t s_handleIrradianceGridSize;
render::handle_t s_handleIrradianceGridSBuffer;
render::handle_t s_handleIrradianceGridBoundsMin;
render::handle_t s_handleIrradianceGridBoundsMax;

#pragma pack(1)

struct LightVertex
{
	float pos[2];
	float texCoord[2];
};

#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightRendererDeferred", LightRendererDeferred, Object)

LightRendererDeferred::LightRendererDeferred()
{
	s_handleTime = render::getParameterHandle(L"World_Time");
	s_handleShadowEnable = render::getParameterHandle(L"World_ShadowEnable");
	s_handleReflectionsEnable = render::getParameterHandle(L"World_ReflectionsEnable");
	s_handleIrradianceEnable = render::getParameterHandle(L"World_IrradianceEnable");
	s_handleExtent = render::getParameterHandle(L"World_Extent");
	s_handleProjection = render::getParameterHandle(L"World_Projection");
	s_handleView = render::getParameterHandle(L"World_View");
	s_handleViewInverse = render::getParameterHandle(L"World_ViewInverse");
	s_handleMagicCoeffs = render::getParameterHandle(L"World_MagicCoeffs");
	s_handleScreenMap = render::getParameterHandle(L"World_ScreenMap");
	s_handleDepthMap = render::getParameterHandle(L"World_DepthMap");
	s_handleNormalMap = render::getParameterHandle(L"World_NormalMap");
	s_handleMiscMap = render::getParameterHandle(L"World_MiscMap");
	s_handleColorMap = render::getParameterHandle(L"World_ColorMap");
	s_handleOcclusionMap = render::getParameterHandle(L"World_OcclusionMap");
	s_handleLightDiffuseMap = render::getParameterHandle(L"World_LightDiffuseMap");
	s_handleLightSpecularMap = render::getParameterHandle(L"World_LightSpecularMap");
	s_handleProbeDiffuse = render::getParameterHandle(L"World_ProbeDiffuse");
	s_handleProbeSpecular = render::getParameterHandle(L"World_ProbeSpecular");
	s_handleProbeSpecularMips = render::getParameterHandle(L"World_ProbeSpecularMips");
	s_handleFogDistanceAndDensity = render::getParameterHandle(L"World_FogDistanceAndDensity");
	s_handleFogColor = render::getParameterHandle(L"World_FogColor");
	s_handleLightCount = render::getParameterHandle(L"World_LightCount");
	s_handleShadowMask = render::getParameterHandle(L"World_ShadowMask");
	s_handleShadowMapAtlas = render::getParameterHandle(L"World_ShadowMapAtlas");
	s_handleReflectionMap = render::getParameterHandle(L"World_ReflectionMap");
	s_handleLightSBuffer = render::getParameterHandle(L"World_LightSBuffer");
	s_handleTileSBuffer = render::getParameterHandle(L"World_TileSBuffer");
	s_handleIrradianceGridSize = render::getParameterHandle(L"World_IrradianceGridSize");
	s_handleIrradianceGridSBuffer = render::getParameterHandle(L"World_IrradianceGridSBuffer");
	s_handleIrradianceGridBoundsMin = render::getParameterHandle(L"World_IrradianceGridBoundsMin");
	s_handleIrradianceGridBoundsMax = render::getParameterHandle(L"World_IrradianceGridBoundsMax");
}

bool LightRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!resourceManager->bind(c_lightShader, m_lightShader))
		return false;
	if (!resourceManager->bind(c_reflectionShader, m_reflectionShader))
		return false;
	if (!resourceManager->bind(c_fogShader, m_fogShader))
		return false;

	// Create fullscreen geometry.
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(LightVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(LightVertex, texCoord)));

	m_vertexBufferQuad = renderSystem->createVertexBuffer(vertexElements, 6 * sizeof(LightVertex), false);
	if (!m_vertexBufferQuad)
		return false;

	LightVertex* vertex = (LightVertex*)m_vertexBufferQuad->lock();
	T_FATAL_ASSERT(vertex);

	vertex[0].pos[0] = -1.0f; vertex[0].pos[1] =  1.0f; vertex[0].texCoord[0] = 0.0f; vertex[0].texCoord[1] = 0.0f;
	vertex[1].pos[0] =  1.0f; vertex[1].pos[1] =  1.0f; vertex[1].texCoord[0] = 1.0f; vertex[1].texCoord[1] = 0.0f;
	vertex[2].pos[0] =  1.0f; vertex[2].pos[1] = -1.0f; vertex[2].texCoord[0] = 1.0f; vertex[2].texCoord[1] = 1.0f;
	vertex[3].pos[0] = -1.0f; vertex[3].pos[1] =  1.0f; vertex[3].texCoord[0] = 0.0f; vertex[3].texCoord[1] = 0.0f;
	vertex[4].pos[0] =  1.0f; vertex[4].pos[1] = -1.0f; vertex[4].texCoord[0] = 1.0f; vertex[4].texCoord[1] = 1.0f;
	vertex[5].pos[0] = -1.0f; vertex[5].pos[1] = -1.0f; vertex[5].texCoord[0] = 0.0f; vertex[5].texCoord[1] = 1.0f;

	m_vertexBufferQuad->unlock();

	m_primitivesQuad.setNonIndexed(render::PtTriangles, 0, 2);
	return true;
}

void LightRendererDeferred::destroy()
{
	safeDestroy(m_vertexBufferQuad);
}

void LightRendererDeferred::renderLights(
	render::RenderContext* renderContext,
	float time,
	int32_t lightCount,
	const Matrix44& projection,
	const Matrix44& view,
	render::StructBuffer* lightSBuffer,
	render::StructBuffer* tileSBuffer,
	IrradianceGrid* irradianceGrid,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	render::ITexture* miscMap,
	render::ITexture* colorMap,
	render::ITexture* occlusionMap,
	render::ITexture* shadowMask,
	render::ITexture* shadowMapAtlas,
	render::ITexture* reflectionMap
)
{
	auto lrb = renderContext->alloc< render::LambdaRenderBlock >();
	lrb->lambda = [=](render::IRenderView* renderView)
	{
		Scalar p11 = projection.get(0, 0);
		Scalar p22 = projection.get(1, 1);

		m_lightShader->setCombination(s_handleShadowEnable, bool(shadowMask != nullptr && shadowMask != nullptr));
		m_lightShader->setCombination(s_handleReflectionsEnable, bool(reflectionMap != nullptr));
		m_lightShader->setCombination(s_handleIrradianceEnable, bool(irradianceGrid != nullptr));

		m_lightShader->setFloatParameter(s_handleTime, time);
		m_lightShader->setFloatParameter(s_handleLightCount, float(lightCount));
		m_lightShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_lightShader->setMatrixParameter(s_handleViewInverse, view.inverse());
		m_lightShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_lightShader->setTextureParameter(s_handleNormalMap, normalMap);
		m_lightShader->setTextureParameter(s_handleMiscMap, miscMap);
		m_lightShader->setTextureParameter(s_handleColorMap, colorMap);
		m_lightShader->setTextureParameter(s_handleOcclusionMap, occlusionMap);
		m_lightShader->setTextureParameter(s_handleShadowMask, shadowMask);
		m_lightShader->setTextureParameter(s_handleShadowMapAtlas, shadowMapAtlas);
		m_lightShader->setTextureParameter(s_handleReflectionMap, reflectionMap);
		m_lightShader->setStructBufferParameter(s_handleLightSBuffer, lightSBuffer);
		m_lightShader->setStructBufferParameter(s_handleTileSBuffer, tileSBuffer);

		if (irradianceGrid)
		{
			const auto size = irradianceGrid->getSize();
			m_lightShader->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0], (float)size[1], (float)size[2], 0.0f));
			m_lightShader->setVectorParameter(s_handleIrradianceGridBoundsMin, irradianceGrid->getBoundingBox().mn);
			m_lightShader->setVectorParameter(s_handleIrradianceGridBoundsMax, irradianceGrid->getBoundingBox().mx);
			m_lightShader->setStructBufferParameter(s_handleIrradianceGridSBuffer, irradianceGrid->getBuffer());
		}

		m_lightShader->draw(renderView, m_vertexBufferQuad, 0, m_primitivesQuad);
	};
	renderContext->enqueue(lrb);
}

void LightRendererDeferred::renderReflections(
	render::RenderContext* renderContext,
	const Matrix44& projection,
	const Matrix44& view,
	const Matrix44& lastView,
	render::ITexture* screenMap,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	render::ITexture* miscMap
)
{
	auto lrb = renderContext->alloc< render::LambdaRenderBlock >();
	lrb->lambda = [=](render::IRenderView* renderView)
	{
		Scalar p11 = projection.get(0, 0);
		Scalar p22 = projection.get(1, 1);

		m_reflectionShader->setMatrixParameter(s_handleProjection, projection);
		m_reflectionShader->setMatrixParameter(s_handleViewInverse, view.inverse());
		m_reflectionShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_reflectionShader->setTextureParameter(s_handleScreenMap, screenMap);
		m_reflectionShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_reflectionShader->setTextureParameter(s_handleNormalMap, normalMap);
		m_reflectionShader->setTextureParameter(s_handleMiscMap, miscMap);

		m_reflectionShader->draw(renderView, m_vertexBufferQuad, 0, m_primitivesQuad);
	};
	renderContext->enqueue(lrb);
}

void LightRendererDeferred::renderFog(
	render::RenderContext* renderContext,
	const Matrix44& projection,
	const Matrix44& view,
	const Vector4& fogDistanceAndDensity,
	const Vector4& fogColor,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	render::ITexture* miscMap,
	render::ITexture* colorMap
)
{
	auto lrb = renderContext->alloc< render::LambdaRenderBlock >();
	lrb->lambda = [=](render::IRenderView* renderView)
	{
		Scalar p11 = projection.get(0, 0);
		Scalar p22 = projection.get(1, 1);

		m_fogShader->setMatrixParameter(s_handleProjection, projection);
		m_fogShader->setMatrixParameter(s_handleView, view);
		m_fogShader->setMatrixParameter(s_handleViewInverse, view.inverse());
		m_fogShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_fogShader->setVectorParameter(s_handleFogDistanceAndDensity, fogDistanceAndDensity);
		m_fogShader->setVectorParameter(s_handleFogColor, fogColor);
		m_fogShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_fogShader->setTextureParameter(s_handleNormalMap, normalMap);
		m_fogShader->setTextureParameter(s_handleMiscMap, miscMap);
		m_fogShader->setTextureParameter(s_handleColorMap, colorMap);

		m_fogShader->draw(renderView, m_vertexBufferQuad, 0, m_primitivesQuad);
	};
	renderContext->enqueue(lrb);
}

	}
}
