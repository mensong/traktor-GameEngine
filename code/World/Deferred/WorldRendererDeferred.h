#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

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

class ImageProcess;
class IRenderTargetSet;
class ISimpleTexture;
class RenderContext;
class RenderGraph;
class StructBuffer;

	}

	namespace world
	{

class GroupEntity;
class IrradianceGrid;
class IShadowProjection;
class LightRendererDeferred;
class WorldEntityRenderers;

struct LightShaderData;
struct TileShaderData;

/*! World renderer, using deferred rendering method.
 * \ingroup World
 *
 * Advanced deferred rendering path.
 *
 * Operation
 * \TBD
 *
 * Techniques used
 * \TBD
 */
class T_DLLCLASS WorldRendererDeferred : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererDeferred();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void attach(Entity* entity) override final;

	virtual void build(const WorldRenderView& worldRenderView, int32_t frame) override final;

	virtual void render(render::IRenderView* renderView, int32_t frame) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct Frame
	{
		Ref< render::RenderContext > renderContext;
		Ref< render::StructBuffer > lightSBuffer;
		Ref< render::StructBuffer > tileSBuffer;
		int32_t lightCount;
	};

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;

	Quality m_toneMapQuality;
	Quality m_motionBlurQuality;
	Quality m_shadowsQuality;
	Quality m_reflectionsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::RenderGraph > m_renderGraph;

	Ref< render::ImageProcess > m_colorTargetCopy;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< render::ImageProcess > m_motionBlurPrimeImageProcess;
	Ref< render::ImageProcess > m_motionBlurImageProcess;
	Ref< render::ImageProcess > m_toneMapImageProcess;
	Ref< render::ImageProcess > m_shadowMaskProject;

	Ref< LightRendererDeferred > m_lightRenderer;

	resource::Proxy< IrradianceGrid > m_irradianceGrid;
	
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
	AlignedVector< Light > m_lights;

	float m_slicePositions[MaxSliceCount + 1];
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;

	void buildGBuffer(const WorldRenderView& worldRenderView);

	void buildVelocity(const WorldRenderView& worldRenderView);

	void buildAmbientOcclusion(const WorldRenderView& worldRenderView);

	void buildLights(const WorldRenderView& worldRenderView, int32_t frame, LightShaderData* lightShaderData, TileShaderData* tileShaderData);

	void buildShadowMask(const WorldRenderView& worldRenderView);

	void buildReflections(const WorldRenderView& worldRenderView);

	void buildVisual(const WorldRenderView& worldRenderView, int32_t frame);

	void buildCopyFrame(const WorldRenderView& worldRenderView);

	void buildEndFrame(const WorldRenderView& worldRenderView);
};

	}
}
