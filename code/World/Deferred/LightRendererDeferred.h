#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/WorldTypes.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;
class ITexture;
class RenderContext;
class Shader;
class StructBuffer;
class VertexBuffer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IrradianceGrid;

/*! Light renderer.
 * \ingroup World
 *
 * Render light primitives for deferred rendering
 * technique.
 */
class LightRendererDeferred : public Object
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	/*! */

	/*! Render all lights. */
	void renderLights(
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
	);

	/*! Render screenspace reflections. */
	void renderReflections(
		render::RenderContext* renderContext,
		const Matrix44& projection,
		const Matrix44& view,
		const Matrix44& lastView,
		render::ITexture* screenMap,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		render::ITexture* miscMap
	);

	/*! Render fog. */
	void renderFog(
		render::RenderContext* renderContext,
		const Matrix44& projection,
		const Matrix44& view,
		const Vector4& fogDistanceAndDensity,
		const Vector4& fogColor,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		render::ITexture* miscMap,
		render::ITexture* colorMap
	);

private:
	resource::Proxy< render::Shader > m_lightShader;
	resource::Proxy< render::Shader > m_reflectionShader;
	resource::Proxy< render::Shader > m_fogShader;
	Ref< render::VertexBuffer > m_vertexBufferQuad;
	render::Primitives m_primitivesQuad;
};

	}
}

